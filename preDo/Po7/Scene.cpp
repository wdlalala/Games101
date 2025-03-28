//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // 最初的射线求交
    Intersection inter = intersect(ray);
    // 如果交点即发光点 返回光强度
    if (inter.m->hasEmission()) return inter.m->getEmission();
    if (!inter.happened) return Vector3f();
    // 采样光源
    float pdf_light;
    Intersection lightInter;
    sampleLight(lightInter, pdf_light);

    // 直接光照
    Vector3f L_dir(0);
	// 间接光照
    Vector3f L_indir(0);

	// 光线方向 = 光源位置 - 交点位置
    Vector3f lightDir = normalize(lightInter.coords - inter.coords);
	// 光源到焦点的距离
    float lightDistance = (lightInter.coords - inter.coords).norm();
	// 光线
    Ray lightRay(inter.coords, lightDir);
	// 光线求交
    Intersection lightRayInter = intersect(lightRay);
	// 发生相交 且 交点就是光源本身
    if (lightRayInter.happened && (lightRayInter.coords - lightInter.coords).norm() < EPSILON && pdf_light > EPSILON)
    {
		// 计算直接光照的计算公式 ： L_dir = emit * f * cosθ * cosθ' / r^2 / pdf_light
        L_dir = lightInter.emit * inter.m->eval(ray.direction, lightDir, inter.normal) *
            dotProduct(inter.normal, lightDir) * dotProduct(lightInter.normal, -lightDir) /
            (lightDistance * lightDistance) / pdf_light;
    }

    // 间接光照
    if (get_random_float() < RussianRoulette)
    {
		// 使用BSDF采样光线方向
        Vector3f wi = inter.m->sample(ray.direction, inter.normal);
        Ray indirRay(inter.coords, wi);
        Intersection indirInter = intersect(indirRay);
         
		// 如果光线有交点 且 递归深度小于最大深度 且 焦点的材质没有发光
        if (indirInter.happened && depth < maxDepth && !indirInter.m->hasEmission())
        {
            if(inter.m->pdf(ray.direction, wi, inter.normal) > EPSILON)
			// 计算间接光照的计算公式 ： L_indir = castRay(indirRay, depth + 1) * f * cosθ * cosθ' / pdf / RussianRoulette
            L_indir = castRay(indirRay, depth + 1) * inter.m->eval(ray.direction, wi, inter.normal) *
                dotProduct(inter.normal, wi) / inter.m->pdf(ray.direction, wi, inter.normal) / RussianRoulette;
        }
    }

    return L_dir + L_indir;
}


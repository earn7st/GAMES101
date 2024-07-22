//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include "Material.hpp"

void Scene::buildBVH()
{
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
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }
    // 这里生成了一个随机浮点数 0 < p < 1，结合总的光源大小，实现随机选择光源进行采样
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object *> &objects,
    float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear)
        {
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
    // TO DO Implement Path Tracing Algorithm here

    Vector3f L_dir = 0.0, L_indir = 0.0;

    Intersection p = intersect(ray);

    // 没打到
    if (!p.happened)
    {
        return L_dir;
    }

    // 打到光源
    if (p.m->hasEmission())
    {
        return p.m->getEmission();
    }

    // 打到非光源表面
    Vector3f N = p.normal.normalized();
    Material *m = p.m;
    Vector3f wo = ray.direction;

    float pdf_light;
    Intersection inter_light;
    sampleLight(inter_light, pdf_light);

    Vector3f ws = (inter_light.coords - p.coords).normalized();
    Vector3f NN = inter_light.normal.normalized();
    Vector3f emit = inter_light.emit;

    // 打出一道向光源方向的光
    Ray Obj2Light(p.coords, ws);
    Intersection obstacle = intersect(Obj2Light);
    // 如果之间没有其它物体阻挡（包括光源或非光源物体），体现为距离相同
    if ((inter_light.coords - p.coords).norm() - obstacle.distance < 0.01)
    {
        // 注意方向
        Vector3f eval = p.m->eval(wo, ws, N);
        float cos_theta = dotProduct(N, ws);
        float cos_theta_light = dotProduct(NN, -ws);
        L_dir = emit * eval * cos_theta * cos_theta_light / std::pow(obstacle.distance, 2) / pdf_light;
    }
    
    // L_indir
    float P_RR = get_random_float();
    if (P_RR < RussianRoulette)
    {
        Vector3f wi = m->sample(wo, N).normalized();
        Ray reflect(p.coords, wi);

        Intersection inter = intersect(reflect);
        // 判断打到的物体是否会发光取决于m
        if (inter.happened && !inter.m->hasEmission())
        {
            Vector3f eval = p.m->eval(wo, wi, N);
            float pdf_reflect = p.m->pdf(wo, wi, N);
            float cos_theta = dotProduct(wi, N);
            L_indir = castRay(reflect, depth + 1) * eval * cos_theta / pdf_reflect / RussianRoulette;
        }
    }
    return L_indir + L_dir;
}
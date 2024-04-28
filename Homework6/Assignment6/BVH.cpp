#include "BVH.hpp"

#include <algorithm>
#include <cassert>

BVHAccel::BVHAccel(std::vector<Object *> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)),
      splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode *BVHAccel::recursiveBuild(std::vector<Object *> objects)
{
    BVHBuildNode *node = new BVHBuildNode();
    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1)
    {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2)
    {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else
    {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim)
        {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().x < f2->getBounds().Centroid().x; });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().y < f2->getBounds().Centroid().y; });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().z < f2->getBounds().Centroid().z; });
            break;
        }
        switch (splitMethod)
        {
        case SplitMethod::NAIVE:
        {
            auto beginning = objects.begin();
            auto middling = objects.begin() + (objects.size() / 2);
            auto ending = objects.end();

            auto leftshapes = std::vector<Object *>(beginning, middling);
            auto rightshapes = std::vector<Object *>(middling, ending);

            assert(objects.size() == (leftshapes.size() + rightshapes.size()));

            node->left = recursiveBuild(leftshapes);
            node->right = recursiveBuild(rightshapes);

            node->bounds = Union(node->left->bounds, node->right->bounds);
        }
        break;

        case SplitMethod::SAH:
        {
            // 这是一种简化版本的SAH（网上最多的版本），对objects内的对象进行桶划分，而不是对空间
            // 正确的SAH应该在三个轴上对空间进行均分，取最小的cost
            // 运行时间9s，比BVH快1~2s
            const int BucketNum = 20;
            int bucketpos = 0;
            const double t_intersect = 1.0f, t_traversal = 0.25f;
            double cost_min = std::numeric_limits<double>::infinity(), cost_tmp = 0;
            double SA = centroidBounds.SurfaceArea();
            for (int i = 1; i <= BucketNum; i++)
            {
                auto beginning = objects.begin();
                auto middling = objects.begin() + objects.size() * i / BucketNum;
                auto ending = objects.end();
                std::vector<Object *> left_shape(beginning, middling);
                std::vector<Object *> right_shape(middling, ending);
                Bounds3 left_bound, right_bound;
                for (int k = 0; k < left_shape.size(); k++)
                {
                    left_bound = Union(left_bound, left_shape[k]->getBounds().Centroid());
                }
                for (int k = 0; k < right_shape.size(); k++)
                {
                    right_bound = Union(right_bound, right_shape[k]->getBounds().Centroid());
                }
                double leftSA = left_bound.SurfaceArea();
                double rightSA = right_bound.SurfaceArea();
                cost_tmp = t_traversal + (leftSA / SA) * left_shape.size() * t_intersect + (rightSA / SA) * right_shape.size() * t_intersect;
                if (cost_min > cost_tmp)
                {
                    cost_min = cost_tmp;
                    bucketpos = i;
                }
            }

            auto beginning = objects.begin();
            auto middling = objects.begin() + objects.size() * bucketpos / BucketNum;
            auto ending = objects.end();

            auto leftshapes = std::vector<Object *>(beginning, middling);
            auto rightshapes = std::vector<Object *>(middling, ending);

            assert(objects.size() == (leftshapes.size() + rightshapes.size()));

            node->left = recursiveBuild(leftshapes);
            node->right = recursiveBuild(rightshapes);

            node->bounds = Union(node->left->bounds, node->right->bounds);
        }
        break;
        }
        return node;
    }
}
Intersection BVHAccel::Intersect(const Ray &ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode *node,
                                       const Ray &ray) const
{
    // TODO Traverse the BVH to find intersection
    /*
    Bounds3 bounds;
    BVHBuildNode *left;
    BVHBuildNode *right;
    Object* object;
    */
    Intersection inter;
    // 但实际上在IntersectP里面没用到dirIsNeg，这里无所谓给不给正确的值
    if (!node->bounds.IntersectP(ray, ray.direction_inv, {0, 0, 0}) ||
        node == nullptr)
    {
        return inter;
    }
    // 到叶子结点了,计算这里面所有三角形是否碰撞，这里调用的是object的子类Triangle的函数
    // getIntersection
    if (node->left == nullptr && node->right == nullptr)
    {
        return node->object->getIntersection(ray);
    }
    Intersection hit_left = getIntersection(node->left, ray);
    Intersection hit_right = getIntersection(node->right, ray);
    // 由于castRay我们要求最近的一个交点，所以应该取distance(时间)(origin +
    // distance * d)的结点
    return hit_left.distance < hit_right.distance ? hit_left : hit_right;
}
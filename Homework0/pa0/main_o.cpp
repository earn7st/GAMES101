#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<iostream>
#define PI acos(-1)

int main(){

    Eigen::Vector3f P(2.0f, 1.0f, 1.0f);
    Eigen::Matrix3f M_r, M_t;
    M_r << cos(PI / 4), -sin(PI / 4), 0, sin(PI / 4), cos(PI / 4), 0, 0, 0, 1.0f;
    M_t << 1.0f, 0, 1.0f, 0, 1.0f, 2.0f, 0, 0, 1.0f;
    P = M_t * M_r * P;
    std::cout << P << std::endl;

    return 0;
}
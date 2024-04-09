//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    };
    Eigen::Vector3f getColorBilinear(float u, float v){

        auto u_img = u * width;
        auto v_img = (1 - v) * height;  //注意这里v是反的，下面对应也要反过来
        auto color_00 = image_data.at<cv::Vec3b>(std::min(float(height), v_img + 1), u_img),
             color_10 = image_data.at<cv::Vec3b>(std::min(float(height), v_img + 1), std::min(float(width), u_img + 1)),
             color_01 = image_data.at<cv::Vec3b>(v_img, u_img),
             color_11 = image_data.at<cv::Vec3b>(v_img, std::min(float(width), u_img + 1));

        auto u00 = Eigen::Vector3f(color_00[0], color_00[1], color_00[2]),
             u10 = Eigen::Vector3f(color_10[0], color_10[1], color_10[2]),
             u01 = Eigen::Vector3f(color_01[0], color_01[1], color_01[2]),
             u11 = Eigen::Vector3f(color_11[0], color_11[1], color_11[2]);
        float s = u_img - int(u_img), t = v_img - int(v_img);   
        Eigen::Vector3f color_u0 = u00 + s * (u10 - u00),
                        color_u1 = u01 + s * (u11 - u01);
        //因为v是反的，所以int(v_img)实际上是四个点中，所以四个点的位置相当于是
        //(height, ...)
        //u00(int(u_img), int(v_img + 1))   u10(int(u_img + 1), int(v_img + 1))
        //u01(int(u_img), int(v_img))       u01(int(u_img + 1), int(v_img))
        //(0, ...)
        //所以纵向的插值和课上讲的公式要反过来 如下:
        Eigen::Vector3f return_color = color_u1 + t * (color_u0 - color_u1);    

        return return_color;
        
    }

};
#endif //RASTERIZER_TEXTURE_H

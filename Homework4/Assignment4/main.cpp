#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#define NUM 4   //控制点个数
#define AA 0    //是否反走样

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < NUM) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f lerp(const cv::Point2f &a, const cv::Point2f &b, double t){
    cv::Point2f ans = a + t * (b - a);
    return ans;
}
//每层递归都要开vector比较占空间，但是考虑控制点较少可以接受
//不然对于每个t都需要copy原始control_points，运算次数太多
cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, double t) 
{
    // TODO: Implement de Casteljau's algorithm
    if(control_points.size() == 1){
        return control_points.front();
    }
    std::vector<cv::Point2f> points;
    for(int i = 1; i < control_points.size(); i ++){
        points.push_back(lerp(control_points[i - 1], control_points[i], t));
    }
    return recursive_bezier(points, t);

}

//考虑周围包括自身共九个点，颜色与距离为一次函数关系
//设像素为1*1的正方形，最长距离为(3sqrt(2))/2,最短距离为0
void AA_draw(const cv::Point2f point, cv::Mat &window){
    cv::Point2f central((int)point.x + 0.5f, (int)point.y + 0.5f);
    cv::Point2f adjacent;
    cv::Vec3b color(0.0f, 0.0f, 0.0f);
    for(int i = -1; i <= 1; i ++){
        for(int j = -1; j <= 1; j ++){
            adjacent.x = central.x + i;
            adjacent.y = central.y + j;
            float distance = std::sqrt((point.x - adjacent.x) * (point.x - adjacent.x) + 
                            (point.y - adjacent.y) * (point.y - adjacent.y));
            float normal_distance = distance * (std::sqrt(2) / 3);
            color[1] = (1 - normal_distance) * 255.0f;
            //我感觉正确的AA应该是记录每个像素被当作临近像素计算过多少次，计算结果取平均
            //不过取最大值的方法应该效果不会差太多
            if(window.at<cv::Vec3b>(adjacent.y, adjacent.x)[1] < color[1]){
                window.at<cv::Vec3b>(adjacent.y, adjacent.x)[1] = color[1];
            }
        }
    }
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    double step = 0.001;
    for(double t = 0.0; t <= 1.0; t += step){
        auto point = recursive_bezier(control_points, t);
        //Anti Aliasing
        AA_draw(point, window);
        //window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
    }

}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == NUM) 
        {
            naive_bezier(control_points, window);
            bezier(control_points, window); 

            cv::imshow("Bezier Curve", window);
            if(!AA) cv::imwrite("my_bezier_curve.png", window);
            else cv::imwrite("AA_my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}

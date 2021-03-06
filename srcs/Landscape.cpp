#include <iostream>
#include <InputManager.hpp>
#include "../includes/Landscape.hpp"

Landscape::Landscape() : _width(50), _height(50)
{
    return ;
};


Landscape::~Landscape(void) {

}

void Landscape::initMap(std::string file)
{
    std::ifstream fs;
    std::string str;
    Vertex3 point;
    std::vector <Vertex3> tab;
    fs.open(file.c_str());
    if (!fs) {
        std::cout << "NO FILE FOUND, ERROR" << std::endl;
        exit(0);
    }
    nb_heights = 4;
    point.xyz = vec3(0, 0, 0);
    tab.push_back(point);
    point.xyz = vec3(0, 0, 49);
    tab.push_back(point);
    point.xyz = vec3(49, 0, 49);
    tab.push_back(point);
    point.xyz = vec3(49, 0, 0);
    tab.push_back(point);
    this->maxHeight = 0.0001;

    while (std::getline(fs, str)) {
        std::string tmp;
        int index;

        if (str == "") {
            continue;
        }

        if (str.substr(0, 2) == "//") {
            continue;
        }

        index = str.find(' ');
        tmp = str.substr(0, index);
        point.xyz.x = std::atoi(tmp.c_str());
        str.erase(0, index + 1);

        index = str.find(' ');
        tmp = str.substr(0, index);
        point.xyz.y = std::atoi(tmp.c_str());
        str.erase(0, index + 1);

        if (point.xyz.y > this->maxHeight) {
            this->maxHeight = point.xyz.y;
        }

        index = str.find(' ');
        tmp = str.substr(0, index);
        point.xyz.z = std::atoi(tmp.c_str());
        str.erase(0, index + 1);
        tab.push_back(point);
        nb_heights += 1;
    }
    fs.close();
    this->generatePlan(tab);
}


Landscape &Landscape::operator=(Landscape const &ref) {
    this->_width = ref.getWidth();
    this->_height = ref.getHeight();
    return *this;

}

int Landscape::getWidth(void) const {
    return this->_width;
}

int Landscape::getHeight(void) const {
    return this->_height;
}

float distance(Vertex3 a, int x, int z) {
    return sqrt(pow((a.xyz.x - x), 2) + pow((a.xyz.z - z), 2));
}

double Landscape::hauteur(std::vector<Vertex3> points, int x, int z) {
    float sum1 = 0;
    float sum2 = 0;

    int i;


    for(i = 0; i < nb_heights; i++){
        float d = distance(points[i], x, z);
        if (x == points[i].xyz.x && z == points[i].xyz.z) {
            return points[i].xyz.y;
        }
        float w = 1 / pow(d, 1.5);
        sum1 = sum1 + (points[i].xyz.y * w);
        sum2 = sum2 + w;
    }
    return sum1 / sum2;
}

Vertex3 Landscape::pushPoint(int x, float y, int z) {
    Vertex3 point;
    float heightColor = y / maxHeight;
    point.xyz = vec3(x, y, z);
    point.rgba = vec4(vec4(0.5f,  1.5f * (1 - heightColor),heightColor * 1.8f, 1));
    return point;

}

void Landscape::generatePlan(std::vector < Vertex3 > points) {

    std::vector <Vertex3> tab;

    heights.resize(_height);
    for(int i = 0; i < 50 ; i++)
    {
        heights[i].resize(_width, 0.0);
    }

    for (int x = 0; x < this->_width - 1; x++) {
     for (int z = 0; z < this->_height - 1; z++) {


         heights[x][z] = hauteur(points, x, z);
         //std::cout << heights[x][z] << std::endl;
         // first triangle
            tab.push_back(pushPoint(x, hauteur(points, x, z), z));
            tab.push_back(pushPoint(x + 1, hauteur(points, x + 1, z), z));
            tab.push_back(pushPoint(x, hauteur(points, x, z - 1), z - 1));
            tab.push_back(pushPoint(x + 1, hauteur(points, x + 1, z), z));
            tab.push_back(pushPoint(x + 1, hauteur(points, x + 1, z - 1), z - 1));
            tab.push_back(pushPoint(x, hauteur(points, x, z - 1), z - 1));

        }
    }

    this->vertab = &tab[0];
    this->size = tab.size();
}
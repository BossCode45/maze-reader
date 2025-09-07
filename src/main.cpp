#include <algorithm>
#include <cstring>
#include <image.h>
#include <iostream>
#include <PNGImage.h>
#include <BMPImage.h>

#include "pathfind.h"

using std::cout, std::endl;

inline int brightness(Pixel& p)
{
	return (p.r+p.r+p.r+p.b+p.g+p.g+p.g+p.g)>>3;
}

void addBlock(Image& image, Point p, Pixel color)
{
	for(int yd = -2; yd < 3; yd++)
		for(int xd = -2; xd < 3; xd++)
			image[p.x+xd, p.y+yd] = color;
}

bool testLine(Image& image, int sX, int sY, int dX, int dY, int dist)
{
	for(int i = 0; i < dist; i++)
	{
		if(sX + dX*i >= 1920 || sX + dX*i < 0 || sY + dY*i >= 1080 || sY + dY*i < 0)
			continue;
		if(brightness(image[sX + dX*i, sY + dY*i])<50)
			return false;
	}
	return true;
}

void line(Image& image, Point start, Point end, Pixel color)
{
	int dX = end.x - start.x;
	int dY = end.y - start.y;
	int dist = std::max(std::abs(dX), std::abs(dY));
	if(dX != 0)
		dX = (dX>0)?1:-1;
	if(dY != 0)
		dY = (dY>0)?1:-1;
	// cout << dX << " " << dY << " " << dist;
	for(int i = 0; i < dist; i++)
	{
		if(start.x + dX*i >= 1920 || start.x + dX*i < 0 || start.y + dY*i >= 1080 || start.y + dY*i < 0)
			continue;
		image[start.x + dX*i, start.y + dY*i] = color;
	}
}

int main(int argc, char* argv[])
{
	//ZLibInflator zlib;
	//zlib.test();

	if(argc < 2)
	{
		cout << "usage: " << argv[0] << " <image>" << endl;
		return 1;
	}
	
	std::string filename = argv[1];

	PNGImage image;
	image.readFromFile(filename);
	const int w = 1920;
	const int h = 1080;
	const int dist = 24;
	std::vector<Node> field;
	field.reserve((h/dist)*(w/dist));
	for(int y = 0; y < h/dist; y++)
	{
		for(int x = 0; x < w/dist; x++)
		{
			Node n = {{x*dist, y*dist}, std::vector<int>()};
			if(x+1 < w/dist && testLine(image, x*dist, y*dist, 1, 0, dist)) // Right
				n.connections.push_back(field.size() + 1);
			if(x > 0 && testLine(image, x*dist, y*dist, -1, 0, dist)) // Left
				n.connections.push_back(field.size() - 1);
			if(y+1 < h/dist && testLine(image, x*dist, y*dist, 0, 1, dist)) // Down
				n.connections.push_back(field.size() + w/dist);
			if(y > 0 && testLine(image, x*dist, y*dist, 0, -1, dist)) // Up
				n.connections.push_back(field.size() - w/dist);
			field.push_back(n);
		}
	}

	BMPImage bmp(image);
	// line(bmp, {912, 552}, {912, 528}, {255, 0, 0});
	// bmp.writeToFile("tmp.bmp");
	// return 0;
	
	for(Node& n : field)
	{
		int i = 0;
		for(int connection : n.connections)
		{
			if(connection < 0 || connection >= field.size())
			{
				// cout << n.p.x << " " << n.p.y << " " << connection << " " << endl;
				
				n.connections[i] = n.connections[n.connections.size() - 1];
				n.connections.erase(n.connections.begin() + n.connections.size() - 1);
				continue;
			}
			line(bmp, n.p, field[connection].p, {0, 0, 255});
			++i;
		}
	}

	Point start = {48, 48};
	Point end = {960, 528};

	addBlock(bmp, start, {0, 255, 0});
	addBlock(bmp, end, {0, 255, 0});

	auto path = pathfind(field, start, end);
	for(int i = 0; i < path.size() - 1; i++)
	{
		// cout << field[path[i]].p.x << "," << field[path[i]].p.y << " : " << field[path[i+1]].p.x << "," << field[path[i+1]].p.y << endl;
		line(bmp, field[path[i]].p, field[path[i+1]].p, {255, 0, 0});
	}
	
	
	bmp.writeToFile("tmp.bmp");
}

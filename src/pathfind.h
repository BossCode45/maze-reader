#pragma once

#include <cstdlib>
#include <vector>

struct Point
{
	int x,y;

	bool operator==(Point other);
};

struct Node
{
	Point p;
	std::vector<int> connections;
};

struct PathfindPoint
{
	Point p;
	int score;
};

int dist(Point start, Point end);

std::vector<int> pathfind(std::vector<Node> nodes, Point start, Point end);

#include "pathfind.h"

#include <algorithm>
#include <iostream>
#include <vector>

bool Point::operator==(Point other)
{
	return x==other.x && y==other.y;
}

int dist(Point start, Point end)
{
	return std::abs(start.x - end.x) + std::abs(start.y - end.y);
}

std::vector<int> pathfind(std::vector<Node> nodes, Point start, Point end)
{
	if(start == end)
		return std::vector<int>();
	const int totalNodes = nodes.size();
	std::vector<int> nodePathDist;
	nodePathDist.reserve(totalNodes);
	std::vector<int> prevNode;
	prevNode.reserve(totalNodes);
	int startID;
	int endID;
	for(int i = 0; i < totalNodes; ++i)
	{
		prevNode.push_back(-1);
		if(nodes[i].p == start)
		{
			nodePathDist.push_back(0);
			startID = i;
		}
		else if(nodes[i].p == end)
		{
			nodePathDist.push_back(-1);
			endID = i;
		}
		nodePathDist.push_back(-1);
	}
	std::vector<int> toCheck = {startID};
	while(prevNode[endID] == -1)
	{
		int bestIndex = 0;
		int bestID = toCheck[bestIndex];
		int bestScore = nodePathDist[bestID] + dist(nodes[bestID].p, end);
		for(int i = 1; i < toCheck.size(); i++)
		{
			int id = toCheck[i];
			if(nodePathDist[id] + dist(nodes[id].p, end) < bestScore)
			{
				bestIndex = i;
				bestID = id;
				bestScore = nodePathDist[bestID] + dist(nodes[bestID].p, end);
			}
		}
		toCheck[bestIndex] = toCheck[toCheck.size() - 1];
		toCheck.erase(toCheck.begin() + toCheck.size() - 1);

		// std::cout << nodes[bestID].p.x << " " << nodes[bestID].p.y << std::endl;
		
		for(int& id : nodes[bestID].connections)
		{
			if(id < 0)
				continue;
			else if(id == startID)
				continue;

			int newDist = nodePathDist[bestID] + 1;
			// std::cout << nodes[id].p.x << " " << nodes[id].p.y << " " << newDist << " " << nodePathDist[id] << std::endl;
			if(nodePathDist[id] == -1 || newDist < nodePathDist[id])
			{
				nodePathDist[id] = newDist;
				prevNode[id] = bestID;
				toCheck.push_back(id);
			}
		}
	}

	std::vector<int> path;
	int currID = endID;
	while(currID != -1)
	{
		path.push_back(currID);
		currID = prevNode[currID];
	}

	std::reverse(path.begin(), path.end());

	return path;
}

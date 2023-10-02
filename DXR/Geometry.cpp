#include "Geometry.h"

void Geometry::CreateSphere(std::vector<Vertex>& vertexPostion, std::vector<UINT>& vertexIndex)
{
	const unsigned int X_SEGMENTS = 64;
	const unsigned int Y_SEGMENTS = 64;

	vertexPostion.clear();
	vertexPostion.reserve(64 * 64);
	for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
	{
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			Vertex vertex;
			float xSegment = float(x) / float(X_SEGMENTS);
			float ySegment = float(y) / float(Y_SEGMENTS);
			vertex.position.x = std::cos(xSegment * 2.0f * std::_Pi) * std::sin(ySegment * std::_Pi);
			vertex.position.y = std::cos(ySegment * std::_Pi);
			vertex.position.z = std::sin(xSegment * 2.0f * std::_Pi) * std::sin(ySegment * std::_Pi);

			vertex.color.x = vertex.position.x;
			vertex.color.y = vertex.position.y;
			vertex.color.z = vertex.position.z;
			vertex.color.w = 1.0;

			vertexPostion.emplace_back(vertex);
		}
	}

	vertexIndex.clear();
	vertexIndex.reserve(X_SEGMENTS * Y_SEGMENTS);
	bool oddRow(false);
	for (unsigned int y = 0; y < Y_SEGMENTS; y++)
	{
		if (!oddRow)
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; x++)
			{
				vertexIndex.push_back(y * (X_SEGMENTS + 1) + x);
				vertexIndex.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else
		{
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
				vertexIndex.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				vertexIndex.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}
}

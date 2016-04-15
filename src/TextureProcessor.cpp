#include "TextureProcessor.h"

#include <algorithm> // swap

#include "slicer/SlicerSegment.h"

namespace cura 
{

#define POINT_DIST 400
#define AMPLITUDE 3000
#define EXTRA_OFFSET 3000

/*
void TextureProcessor::process(std::vector< Slicer* >& slicer_list)
{
    for (Slicer* slicer : slicer_list)
    {
        for (SlicerLayer& layer : slicer->layers)
        {
            process(slicer->mesh, layer);
        }
    }
}
*/

void TextureProcessor::process(Mesh* mesh, SlicerLayer& layer)
{
    Polygons results;
    for (PolygonRef poly : layer.polygonList)
    {
        // generate points in between p0 and p1
        PolygonRef result = results.newPoly();
        
        int64_t dist_left_over = (POINT_DIST / 2); // the distance to be traversed on the line before making the first new point
        Point* p0 = &poly.back();
        for (Point& p1 : poly)
        { // 'a' is the (next) new point between p0 and p1
            SlicerSegment segment(*p0, p1);
            auto it = layer.segment_to_material_segment.find(segment);
            if (it != layer.segment_to_material_segment.end())
            {
                MatSegment& mat = it->second;
                MatCoord mat_start = mat.start;
                MatCoord mat_end = mat.end;
                if (it->first.start != *p0)
                {
                    std::swap(mat_start, mat_end);
                }
                Point p0p1 = p1 - *p0;
                Point perp_to_p0p1 = turn90CCW(p0p1);
                int64_t p0p1_size = vSize(p0p1);    
                int64_t dist_last_point = dist_left_over + p0p1_size * 2; // so that p0p1_size - dist_last_point evaulates to dist_left_over - p0p1_size
                // TODO: move start point (which was already moved last iteration
                for (int64_t p0pa_dist = dist_left_over; p0pa_dist < p0p1_size; p0pa_dist += POINT_DIST)
                {
                    MatCoord mat_coord_now = mat_start;
                    mat_coord_now.coords = mat_start.coords + (mat_end.coords - mat_start.coords) * p0pa_dist / p0p1_size;
                    float val = mesh->getColor(mat_coord_now);
                    int r = val * (AMPLITUDE * 2) - AMPLITUDE + EXTRA_OFFSET;
                    Point fuzz = normal(perp_to_p0p1, r);
                    Point pa = *p0 + normal(p0p1, p0pa_dist) - fuzz;
                    result.add(pa);
                    dist_last_point = p0pa_dist;
                }
                // TODO: move end point as well
                float val = mesh->getColor(mat_end);
                int r = val * (AMPLITUDE * 2) - AMPLITUDE + EXTRA_OFFSET;
                Point fuzz = normal(perp_to_p0p1, r);
                result.emplace_back(p1 - fuzz);
                dist_left_over = p0p1_size - dist_last_point;
            }
            else
            {
                result.emplace_back(p1);
            }
            p0 = &p1;
        }
        while (result.size() < 3 )
        {
            unsigned int point_idx = poly.size() - 2;
            result.add(poly[point_idx]);
            if (point_idx == 0) { break; }
            point_idx--;
        }
        if (result.size() < 3)
        {
            result.clear();
            for (Point& p : poly)
                result.add(p);
        }
    }
    layer.polygonList = results;
}



}//namespace cura

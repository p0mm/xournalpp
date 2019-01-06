#include "StrokeView.h"
#include "DocumentView.h"

#include "model/eraser/EraseableStroke.h"
#include "model/Stroke.h"

StrokeView::StrokeView(cairo_t* cr, Stroke* s)
 : cr(cr),
   s(s)
{
}

StrokeView::~StrokeView()
{
}

void StrokeView::drawFillStroke()
{
	ArrayIterator<Point> points = s->pointIterator();

	if (points.hasNext())
	{
		Point p = points.next();
		cairo_move_to(cr, p.x, p.y);
	}
	else
	{
		return;
	}

	while (points.hasNext())
	{
		Point p = points.next();
		cairo_line_to(cr, p.x, p.y);
	}

	cairo_fill(cr);
}

void StrokeView::applyDashed(cairo_t* cr, Stroke* s, double offset)
{
	const double* dashes = NULL;
	int dashCount = 0;
	if (s->getDashes(dashes, dashCount))
	{
		cairo_set_dash(cr, dashes, dashCount, offset);
	}
}

void StrokeView::drawEraseableStroke(cairo_t* cr, Stroke* s)
{
	EraseableStroke* e = s->getEraseable();
	e->draw(cr);
}

/**
 * Change cairo source, used to draw hilighter transparent,
 * but only if not currently drawing and so on (yes, complicated)
 */
void StrokeView::changeCairoSource(bool markAudioStroke)
{
	///////////////////////////////////////////////////////
	// Fill stroke
	///////////////////////////////////////////////////////
	if (s->getFill() != -1 && s->getToolType() != STROKE_TOOL_HIGHLIGHTER)
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

		// Set the color and transparency
		DocumentView::applyColor(cr, s, s->getFill());

		drawFillStroke();
	}


	if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER ||
		(s->getAudioFilename().length() == 0 && markAudioStroke))
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
		// Set the color
		DocumentView::applyColor(cr, s, 120);
	}
	else
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		// Set the color
		DocumentView::applyColor(cr, s);
	}
}

void StrokeView::paint(int startPoint, double scaleFactor, bool noAlpha, bool dontRenderEditingStroke)
{
	ArrayIterator<Point> points = s->pointIterator();

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	// don't render eraseable for previews
	if (s->getEraseable() && !dontRenderEditingStroke)
	{
		drawEraseableStroke(cr, s);
		return;
	}

	int count = 1;
	double width = s->getWidth();

	// No pressure sensitivity, easy draw a line...
	if (!s->hasPressure() || s->getToolType() == STROKE_TOOL_HIGHLIGHTER)
	{
		bool group = false;
		if (s->getFill() != -1 && s->getToolType() == STROKE_TOOL_HIGHLIGHTER)
		{
			cairo_push_group(cr);
			// Do not apply the alpha here, else the border and the fill
			// are visible instead of one homogeneous area
			DocumentView::applyColor(cr, s, 255);
			drawFillStroke();
			group = true;
		}

		// Set width
		cairo_set_line_width(cr, width * scaleFactor);
		//applyDashed(cr, s);

		while (points.hasNext())
		{
			Point p = points.next();

			if (startPoint <= count)
			{
				cairo_line_to(cr, p.x, p.y);
			}
			else
			{
				cairo_move_to(cr, p.x, p.y);
			}

			count++;
		}

		cairo_stroke(cr);

		if (group)
		{
			cairo_pop_group_to_source(cr);

			if (noAlpha)
			{
				// Currently drawing -> transparent applied on blitting
				cairo_paint(cr);
			}
			else
			{
				cairo_paint_with_alpha(cr, s->getFill() / 255.0);
			}
		}
		return;
	}

	///////////////////////////////////////////////////////
	// Presure sensitiv stroken
	///////////////////////////////////////////////////////


	Point lastPoint1(-1, -1);
	lastPoint1 = points.next();

	while (points.hasNext())
	{
		Point p = points.next();
		if (startPoint <= count)
		{
			if (lastPoint1.z != Point::NO_PRESURE)
			{
				width = lastPoint1.z;
			}

			// Set width
			cairo_set_line_width(cr, width * scaleFactor);

			cairo_move_to(cr, lastPoint1.x, lastPoint1.y);
			cairo_line_to(cr, p.x, p.y);
			cairo_stroke(cr);
		}
		count++;
		lastPoint1 = p;
	}

	cairo_stroke(cr);
}


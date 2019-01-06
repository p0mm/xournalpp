/*
 * Xournal++
 *
 * Draw stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

class Stroke;

class StrokeView
{
public:
	StrokeView(cairo_t* cr, Stroke* s);
	~StrokeView();

public:
	void paint(int startPoint, double scaleFactor, bool noAlpha, bool dontRenderEditingStroke);

	/**
	 * Change cairo source, used to draw hilighter transparent,
	 * but only if not currently drawing and so on (yes, complicated)
	 */
	void changeCairoSource(bool markAudioStroke);

private:
	void drawFillStroke();
	void applyDashed(cairo_t* cr, Stroke* s, double offset);
	void drawEraseableStroke(cairo_t* cr, Stroke* s);

private:
	cairo_t* cr;
	Stroke* s;
};

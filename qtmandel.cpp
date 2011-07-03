// qtmandel.cpp
#include <qwidget.h>
#include <qcolor.h>
#include "qtmandel.h"

QTMandel::QTMandel(QWidget *parent, const char *name, 
		   unsigned int iterations, int w, int h) 
  : Mandel(iterations, w, h), QWidget(parent, name)

{
   // man ska inte kunna ändra storleken
   setMinimumSize(Mandel::width, Mandel::height); 
   setMaximumSize(Mandel::width, Mandel::height);

   // Kopiera paletten
   int i;
   for (i = 0; i<256; i++) {
      qt_palette[i] = QColor(m_palette[0][i], 
			    m_palette[1][i], m_palette[2][i]);
   }
   
   // Svart bakgrund i widgeten. Funktion från QWidget
   setBackgroundColor(qt_palette[0]);
   pix.resize(Mandel::width, Mandel::height);
   // optimera kopieringen till pixmapen
   pix.setOptimization(QPixmap::BestOptim); 
   pix.fill(qt_palette[0]); // svart bakgrund i pixmapen
}

void QTMandel::paintEvent(QPaintEvent *)
{
   // visa pixmapen t ex om fönstret behöver ritas om
   if (havepainted)
     bitBlt(this, 0, 0, &pix);
   else {
      paint.begin(&pix);
      calcmandel();
      paint.end();
   }
}

void QTMandel::mousePressEvent(QMouseEvent * mevent)
{
	if (mevent->button() == LeftButton) {
		fromx = mevent->x();
		fromy = mevent->y();
	}
	
   if (mevent->button() == RightButton) { // zooma tillbaka ett steg
		paint.begin(&pix);
		zoom_back();
		paint.end();
	}
	
   if (mevent->button() == MidButton) { // zooma tillbaka till start-bilden
		paint.begin(&pix);
		zoom_start();
		paint.end();
	}
}

void QTMandel::mouseReleaseEvent(QMouseEvent * mevent)
{
   int tox, toy;

   if (mevent->button() == LeftButton) {
      tox = mevent->x();
      toy = mevent->y();
      
      paint.begin(&pix);
      zoom_cord(fromx, fromy, tox, toy);
      paint.end();
   }
}

// Den här funktionen är tänkt att rita en rektangel
// när man håller ner en musknapp.
// Av någon anledning vägrar QT att registrera att man
// släpper knappen om man målar rektangeln, därför sker heller ingen zoomning
void QTMandel::mouseMoveEvent(QMouseEvent * mevent)
{
   /*
   repaint();
   paint.begin(this);
   paint.setPen(QColor(255,255,255));
   paint.drawRect(fromx, fromy, mevent->x() - fromx, mevent->y() - fromy);
   paint.end();
   */
}

void QTMandel::plot(int x, int y, int color)
{
   paint.setPen(qt_palette[color]);
   paint.drawPoint(x, y);
   bitBlt(this, x, y, &pix, x, y, 1, 1); // kopiera pixeln till bufferten pix
}

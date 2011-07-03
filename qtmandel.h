// qtmandel.h
#ifndef _qtmandel_h_
#define _qtmandel_h_
#include <qwidget.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qpainter.h>
#include "mandel.h"
class QTMandel : public Mandel, public QWidget
{
 public:
   QTMandel( QWidget *parent=0, const char *name=0,
		unsigned int iterations = 100, int w = 640, int h = 480);
      
 protected:
   // QT-Events anropas automatiskt av QT
   void paintEvent( QPaintEvent *); // rita fönstret
   void mousePressEvent(QMouseEvent * mevent); // hanterar musknappstryckning
   void mouseReleaseEvent(QMouseEvent * mevent); // släppning av muskknappar
   void mouseMoveEvent(QMouseEvent * mevent); // ritar upp zoom-rektangel
   
   void plot(int x, int y, int color); // overrides base-class
   void repaint();
 private:
   // Array med färgerna. Borde ha hanterats internt av klassen
   // men har inte tid att implementera detta.
   QColor qt_palette[256]; 

   QPixmap pix; // en buffert för kalkylerad bild
   QPainter paint; // används för att rita i widgeten

   int fromx, fromy; // temporära variabler för att zooma
};
#endif // _qtmandel_h_

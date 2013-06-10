/*
 * Copyright (c) 2012-2012 Wei Song <songw@cs.man.ac.uk> 
 *    Advanced Processor Technologies Group, School of Computer Science
 *    University of Manchester, Manchester M13 9PL UK
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* 
 * a command line tool to convert sdfg to pdf
 * 22/09/2012   Wei Song
 *
 *
 */

#include "sdfg2pdf.hpp"

using std::string;
using boost::shared_ptr;
using std::list;
using std::pair;

static const double MIN_ROUND_DIST = 10.0;
static const double TOKEN_DIST = 5;
static const double ARROW_SIZE = MIN_ROUND_DIST/2;
static const char * PDF_FONT = "times";
static const unsigned int PDF_FONT_SIZE = 12;
static const double PI = 3.14159265;
static const double PDF_PEN_SIZE = 1.5;

int main(int argc, char *argv[]) {

  if(argc != 2) return 1;
  string filename(argv[1]);

  QApplication app(argc, argv, false);
  
  std::ifstream infile(filename.c_str());
  shared_ptr<SDFG::dfgGraph> G = SDFG::read(infile);
  infile.close();

  filename += ".pdf";

  QPrinter printer(QPrinter::ScreenResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(filename.c_str());
  printer.setPaperSize(QPrinter::A4);  // default A4
  printer.setColorMode(QPrinter::Color); // explicitly set it to color mode

  QPainter painter;
  if(!painter.begin(&printer)) return 3;

  // set the pen
  QFont font(PDF_FONT, PDF_FONT_SIZE);
  painter.setFont(font);
  QPen pen;
  pen.setWidthF(PDF_PEN_SIZE);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);

  std::set<string> module_set;
  std::list<shared_ptr<SDFG::dfgGraph> > glist;

  module_set.insert(G->name);
  shared_ptr<SDFG::dfgGraph> gg = G;

  while(true) {
    draw_page(*gg, painter, printer, module_set, glist);
    if(glist.empty())
      break;
    else {
      //printer.newPage();
      gg = glist.front();
      glist.pop_front();
    }
  }

  painter.end();
  
  filename = "debug.sdfg";
  std::ofstream outfile(filename.c_str());
  G->write(outfile);
  outfile.close();
  
  return 0;
}



void draw_page(SDFG::dfgGraph& g, QPainter& painter, QPrinter& printer, std::set<string>& module_set, list<shared_ptr<SDFG::dfgGraph> >& glist) {
  QPen pen = painter.pen();
  QFont font = painter.font();
  //QTransform transform = painter.transform();

  typedef pair<double, double> PType;

  // generate OGDF graph
  ogdf::Graph og;
  ogdf::GraphAttributes ga;
  g.write(&og, &ga);

  // do the layout
  g.layout(&og, &ga);

  // scale it
  // calculate bounding box
  QRectF bound(0.0, 0.0, 0.0, 0.0);
  for_each(g.nodes.begin(), g.nodes.end(), 
           [&](pair<const SDFG::vertex_descriptor, shared_ptr<SDFG::dfgNode> >& m) {
             SDFG::dfgNode& node = *(m.second);
             if(bound.width() < node.position.first + node.bbox.first/2.0) 
               bound.setWidth(node.position.first + node.bbox.first/2.0);
             if(bound.height() < node.position.second + node.bbox.second/2.0)
               bound.setHeight(node.position.second + node.bbox.second/2.0);
           });
  
  for_each(g.edges.begin(), g.edges.end(), 
           [&](pair<const SDFG::edge_descriptor, shared_ptr<SDFG::dfgEdge> >& m) {
             SDFG::dfgEdge& edge = *(m.second);
             for_each(edge.bend.begin(), edge.bend.end(),
                      [&](pair<double, double>& m) {
                        if(bound.width() < m.first) 
                          bound.setWidth(m.first);
                        if(bound.height() < m.second)
                          bound.setHeight(m.second);
                      });
           });

  //ogdf::DRect ogdf_bbox = ga.boundingBox();
  /*
  QRectF bound(0.0, 0.0, ogdf_bbox.width(), ogdf_bbox.height());
  QRect window = painter.window();
  double sr = 1.0 / std::max(bound.height()/window.width(), bound.width()/window.height());
  if(sr < 1 && bound.width() > bound.height()) { // landscape
    sr = 1.0 / std::max(bound.height()/window.width(), bound.width()/window.height());
    if(sr < 1)
      painter.setTransform( QTransform(0, sr, -sr, 0, window.width(), 0), true);
      
  } else if(sr < 1) { 
    painter.setTransform( QTransform(sr, 0, 0, sr, 0, 0), true);
  }
  */
  // change size
  QSizeF psize(bound.width()+200, bound.height()+200);
  printer.setPaperSize(psize, QPrinter::DevicePixel);
  printer.setPageMargins(100, 100, 100, 100, QPrinter::DevicePixel);
  printer.newPage();

  // draw the pdf
  // the name of the module
  painter.drawText(0.0, 0.0, ("module: " + g.name).c_str());

  // nodes
  for_each(g.nodes.begin(), g.nodes.end(), 
           [&](pair<const SDFG::vertex_descriptor, shared_ptr<SDFG::dfgNode> >& m) {
             SDFG::dfgNode& node = *(m.second);
             double h, w, x, y;
             boost::tie(w, h) = node.bbox;
             boost::tie(x, y) = node.position;
             x -= w/2;
             y -= h/2;

             QRectF bbox = QRectF(x, y, w, h);
             QPoint name_point(x+w-5, y-5);

             switch(node.type) {
             case SDFG::dfgNode::SDFG_COMB: {
               painter.drawEllipse(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "combi");
               painter.drawText(name_point, node.name.c_str());
               break;
             }
             case SDFG::dfgNode::SDFG_FF: {
               painter.drawRect(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "FF");
               painter.drawText(name_point, node.name.c_str());
               break;
             }
             case SDFG::dfgNode::SDFG_LATCH: {
               painter.drawRect(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "LAT");
               painter.drawText(name_point, node.name.c_str());
               break;
             }
             case SDFG::dfgNode::SDFG_MODULE: {
               pen.setWidthF(PDF_PEN_SIZE+1); painter.setPen(pen);
               painter.drawRect(bbox);
               pen.setWidthF(PDF_PEN_SIZE); painter.setPen(pen);
               painter.drawText(bbox, Qt::AlignCenter, "module");
               if(!module_set.count(node.child_name))
                 glist.push_back(node.child);
               painter.drawText(name_point, (node.name + "(" + node.child_name + ")").c_str());
               break;
             }
             case SDFG::dfgNode::SDFG_IPORT: {
               painter.drawEllipse(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "I");
               painter.setFont(QFont(PDF_FONT, PDF_FONT_SIZE-4));
               painter.drawText(name_point, node.name.substr(0,8).c_str());
               painter.setFont(font);
               break;
             }
             case SDFG::dfgNode::SDFG_OPORT: {
               painter.drawEllipse(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "O");
               painter.setFont(QFont(PDF_FONT, PDF_FONT_SIZE-4));
               painter.drawText(name_point, node.name.substr(0,8).c_str());
               painter.setFont(font);
               break;
             }
             case SDFG::dfgNode::SDFG_PORT: {
               painter.drawEllipse(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "I/O");
               painter.setFont(QFont(PDF_FONT, PDF_FONT_SIZE-4));
               painter.drawText(name_point, node.name.substr(0,8).c_str());
               painter.setFont(font);
               break;
             }
             default: {
               painter.drawEllipse(bbox);
               painter.setFont(QFont(PDF_FONT, PDF_FONT_SIZE-4));
               painter.drawText(name_point, node.name.substr(0,8).c_str());
               painter.setFont(font);
               break;
             }
             }
           });

  // edges
  for_each(g.edges.begin(), g.edges.end(), 
           [&](pair<const SDFG::edge_descriptor, shared_ptr<SDFG::dfgEdge> >& m) {
             list<PType> line, bend;
             double x0, y0, x1, y1, x2, y2; // temp values
             
             // find out the source and target node
             SDFG::dfgNode s = *(g.get_source(m.second));
             SDFG::dfgNode t = *(g.get_target(m.second));
             line = m.second->bend;
             line.push_front(s.position);
             line.push_back(t.position);

             // correct the start point
             list<PType>::iterator it = line.begin();
             boost::tie(x0, y0) = *it;
             boost::tie(x1, y1) = *(++it);
             double angle = atan2(y1-y0, x1-x0);
             double hw, hh;
             boost::tie(hw,hh) = s.bbox;
             hw /= 2.0;
             hh /= 2.0;

             if(s.type & (SDFG::dfgNode::SDFG_FF | SDFG::dfgNode::SDFG_LATCH | SDFG::dfgNode::SDFG_MODULE)) {
               // rectangle
               QPointF ipoint;
               QLineF ml;
               double rangle = atan2(hh, hw); // the angle of the rect
               if(angle > rangle && angle <= PI-rangle)
                 ml.setLine(x0-hw, y0+hh, x0+hw, y0+hh);
               else if(angle > PI-rangle || angle < -PI + rangle)
                 ml.setLine(x0-hw, y0-hh, x0-hw, y0+hh);
               else if(angle >= -PI + rangle && angle < -rangle )
                 ml.setLine(x0-hw, y0-hh, x0+hw, y0-hh);
               else
                 ml.setLine(x0+hw, y0-hh, x0+hw, y0+hh);
               
               ml.intersect(QLineF(x0, y0, x1, y1), &ipoint);
               line.front() = PType(ipoint.x(), ipoint.y());
             } else {           // ovel
               line.front() = PType(x0+hw*cos(angle), y0+hh*sin(angle));
             } 
             
             // correct the end point
             it = line.end();
             boost::tie(x0, y0) = *(--it);
             boost::tie(x1, y1) = *(--it);
             angle = atan2(y1-y0, x1-x0);
             boost::tie(hw,hh) = t.bbox;
             hw /= 2.0;
             hh /= 2.0;
             if(t.type & (SDFG::dfgNode::SDFG_FF | SDFG::dfgNode::SDFG_LATCH | SDFG::dfgNode::SDFG_MODULE)) {
               // rectangle
               QPointF ipoint;
               QLineF ml;
               double rangle = atan2(hh, hw); // the angle of the rect
               if(angle > rangle && angle <= PI-rangle)
                 ml.setLine(x0-hw, y0+hh, x0+hw, y0+hh);
               else if(angle > PI-rangle || angle < -PI + rangle)
                 ml.setLine(x0-hw, y0-hh, x0-hw, y0+hh);
               else if(angle >= -PI + rangle && angle < -rangle )
                 ml.setLine(x0-hw, y0-hh, x0+hw, y0-hh);
               else
                 ml.setLine(x0+hw, y0-hh, x0+hw, y0+hh);
               
               ml.intersect(QLineF(x0, y0, x1, y1), &ipoint);
               line.back() = PType(ipoint.x(), ipoint.y());
             } else {           // ovel
               line.back() = PType(x0+hw*cos(angle), y0+hh*sin(angle));
             }

             // smooth the bend
             {
               list<PType>::iterator p = line.begin();
               list<PType>::iterator end = line.end();
               while(true) {
                 list<PType>::iterator it = p++;
                 boost::tie(x0, y0) = *it++;
                 boost::tie(x1, y1) = *it++;
                 if(it != end)
                   boost::tie(x2, y2) = *it;
                 else
                   break;
                 double l0 = pow(pow(x1-x0,2)+pow(y1-y0,2),0.5);
                 double l1 = pow(pow(x2-x1,2)+pow(y2-y1,2),0.5);
                 
                 // 0 -> 1
                 if(l0 > MIN_ROUND_DIST*2) {
                   double r = (l0-MIN_ROUND_DIST)/l0;
                   bend.push_back(PType(x0+(x1-x0)*r, y0+(y1-y0)*r));
                 } else {
                   bend.push_back(PType((x0+x1)/2, (y0+y1)/2));
                 }
                 
                 // 1 -> 2
                 if(l1 > MIN_ROUND_DIST*2) {
                   double r = MIN_ROUND_DIST/l1;
                   bend.push_back(PType(x1+(x2-x1)*r, y1+(y2-y1)*r));
                 } else {
                   bend.push_back(PType((x2+x1)/2, (y2+y1)/2));
                 }
               }
             }
             
             QPainterPath dpath;
             dpath.moveTo(line.front().first, line.front().second);
             {
               list<PType>::iterator p = line.begin();
               p++;
               list<PType>::iterator b1 = bend.begin();
               list<PType>::iterator b0 = b1++;
               
               do {
                 if(b0 != bend.end()) {
                   dpath.lineTo(b0->first, b0->second);
                   dpath.quadTo(p->first, p->second, b1->first, b1->second);
                   p++;
                   b0 = ++b1;
                   b1++;
                 } else {
                   dpath.lineTo(p->first, p->second);
                   break;
                 }
               } while(true);
             }
 
             QPen line_pen = pen;
             switch(m.second->type) {
             case SDFG::dfgEdge::SDFG_DF: 
               line_pen.setColor(Qt::black);
               line_pen.setStyle(Qt::SolidLine);
               line_pen.setWidth(PDF_PEN_SIZE + 0);
               break;
             case SDFG::dfgEdge::SDFG_DDP:
               line_pen.setColor(Qt::black);
               line_pen.setStyle(Qt::DotLine);
               line_pen.setWidth(PDF_PEN_SIZE + 0);
               break;
             case SDFG::dfgEdge::SDFG_CAL:
               line_pen.setColor(Qt::black);
               line_pen.setStyle(Qt::SolidLine);
               line_pen.setWidth(PDF_PEN_SIZE + 1.5);
               break;
             case SDFG::dfgEdge::SDFG_ASS:
               line_pen.setColor(Qt::black);
               line_pen.setStyle(Qt::DashDotLine);
               line_pen.setWidth(PDF_PEN_SIZE + 1.5);
               break;
             case SDFG::dfgEdge::SDFG_DAT:
               line_pen.setColor(Qt::black);
               line_pen.setStyle(Qt::DashLine);
               line_pen.setWidth(PDF_PEN_SIZE + 1.5);
               break;
             case SDFG::dfgEdge::SDFG_CTL:
               line_pen.setColor(Qt::darkRed);
               line_pen.setStyle(Qt::SolidLine);
               line_pen.setWidth(PDF_PEN_SIZE + 0);
               break;
             case SDFG::dfgEdge::SDFG_CMP:
               line_pen.setColor(Qt::red);
               line_pen.setStyle(Qt::DashDotLine);
               line_pen.setWidth(PDF_PEN_SIZE + 1.5);
               break;
             case SDFG::dfgEdge::SDFG_EQU:
               line_pen.setColor(Qt::red);
               line_pen.setStyle(Qt::DashLine);
               line_pen.setWidth(PDF_PEN_SIZE + 1.5);
               break;
             case SDFG::dfgEdge::SDFG_CLK:
               line_pen.setColor(Qt::blue);
               line_pen.setStyle(Qt::SolidLine);
               line_pen.setWidth(PDF_PEN_SIZE + 0);
               break;
             case SDFG::dfgEdge::SDFG_RST:
               line_pen.setColor(Qt::green);
               line_pen.setStyle(Qt::SolidLine);
               line_pen.setWidth(PDF_PEN_SIZE + 0);
               break;
             default:
               line_pen.setColor(Qt::black);
             }

             painter.setPen(line_pen);
             painter.drawPath(dpath);
             
             // draw the arrow
             {
               list<PType>::const_reverse_iterator p0 = line.rbegin();
               list<PType>::const_reverse_iterator p1 = p0++;
               painter.fillPath(make_arrow(p0->first, p0->second, 
                                           p1->first, p1->second),
                                QBrush(line_pen.color()));
             }
           });
    
  // reset the pen
  //painter.setTransform(transform);
  painter.setPen(pen);
  painter.setFont(font);
}

QPainterPath make_arrow(double x0, double y0, double x1, double y1) {
  // alway an arrow from 0 -> 1
  QPainterPath m;
  m.moveTo(0,0);
  m.lineTo(-2.2*ARROW_SIZE, ARROW_SIZE);
  m.lineTo(-1.5*ARROW_SIZE, 0);
  m.lineTo(-2.2*ARROW_SIZE, -ARROW_SIZE);
  m.lineTo(0,0);
  m.setFillRule(Qt::WindingFill);

  double angle = atan2(y1-y0, x1-x0);
  QTransform rotateTran(cos(angle), sin(angle), -sin(angle), cos(angle), x1, y1);
  return rotateTran.map(m);
}


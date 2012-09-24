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
static const double ARROW_SIZE = MIN_ROUND_DIST/3;
static const char * PDF_FONT = "times";
static const unsigned int PDF_FONT_SIZE = 12;
static const double PI = 3.14159265;

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

  QPainter painter;
  if(!painter.begin(&printer)) return 3;

  // set the pen
  QFont font(PDF_FONT, PDF_FONT_SIZE);
  painter.setFont(font);
  QPen pen;
  pen.setWidthF(1.5);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);

  std::set<string> module_set;
  std::list<shared_ptr<SDFG::dfgGraph> > glist;

  module_set.insert(G->name);
  shared_ptr<SDFG::dfgGraph> gg = G;

  while(true) {
    draw_page(*gg, painter, module_set, glist);
    if(glist.empty())
      break;
    else {
      printer.newPage();
      gg = glist.front();
      glist.pop_front();
    }
  }

  painter.end();
  
  /*
  filename += ".copy";
  std::ofstream outfile(filename.c_str());
  G->write(outfile);
  outfile.close();
  */

  return 0;
}



void draw_page(SDFG::dfgGraph& g, QPainter& painter, std::set<string>& module_set, list<shared_ptr<SDFG::dfgGraph> >& glist) {
  QPen pen = painter.pen();
  QFont font = painter.font();
  QTransform transform = painter.transform();

  typedef pair<double, double> PType;

  // generate OGDF graph
  ogdf::Graph og;
  ogdf::GraphAttributes ga;
  g.write(&og, &ga);

  // do the layout
  g.layout(&og, &ga);

  // scale it
  ogdf::DRect ogdf_bbox = ga.boundingBox();
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

  // draw the pdf
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
             
             switch(node.type) {
             case SDFG::dfgNode::SDFG_COMB: {
               painter.drawEllipse(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "combi");
               break;
             }
             case SDFG::dfgNode::SDFG_FF: {
               painter.drawRect(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "FF");
               break;
             }
             case SDFG::dfgNode::SDFG_LATCH: {
               painter.drawRect(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "LAT");
               break;
             }
             case SDFG::dfgNode::SDFG_MODULE: {
               painter.drawRect(bbox);
               painter.drawText(bbox, Qt::AlignCenter, "module");
               if(!module_set.count(node.child_name))
                 glist.push_back(node.child);
               break;
             }
             case SDFG::dfgNode::SDFG_IPORT: {
               painter.drawEllipse(bbox);
               break;
             }
             case SDFG::dfgNode::SDFG_OPORT: {
               painter.drawEllipse(bbox);
               break;
             }
             case SDFG::dfgNode::SDFG_PORT: {
               painter.drawEllipse(bbox);
               break;
             }
             default: {
               painter.drawEllipse(bbox);
               break;
             }
             }
           });

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

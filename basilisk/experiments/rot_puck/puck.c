#include "grid/octree.h"
#include "utils.h"
#include "fractions.h"
#include "view.h"
//TODO center drawing function with origin

int main() {
  init_grid(64);
  double L0 = 1.;
  X0 = Y0 = Z0 = 0.;
 
  double R = 0.25;
  double w = 0.10;
  
  double xf = 1.*L0/2.;
  double yf = 1.*L0/2.;
  double zf = 1.*L0/2.;
  scalar fan[], sph[], plane[];
  fan.prolongation = fraction_refine;
  view(theta = 0.1, phi = 0.3);

  double rTheta = 0.;  
  double rPhi = 0.;

  double steps = 40.;
  for(double i=0;i<steps;i++) {
    if(i<=40.){
      rTheta += 2*M_PI/40.;
      rPhi = 0.;
    }
    if( (i>40.) && (i<=80.)){
      rTheta = M_PI/2;
      rPhi += 2*M_PI/40.;
    }
    if(i>80.){
      rTheta += 2*M_PI/40.;
      rPhi += 2*M_PI/40.;
    }

    printf("i=%g, theta=%g, phi=%g \n", i, rTheta, rPhi);
	
    double rn[3] = {sin(rTheta)*cos(-rPhi + M_PI/2.), sin(rTheta)*sin(-rPhi + M_PI/2.), cos(rTheta)};

    fraction(sph, -sq((x - xf)) - sq((y - yf)) - sq((z - zf)) + sq(R));
    fraction(plane, rn[0]*(x-xf) + rn[1]*(y-yf) + rn[2]*(z-zf) + w/2.);
    foreach ()
      fan[] = sph[] * plane[];
    fraction(plane, -rn[0]*(x-xf) - rn[1]*(y-yf) - rn[2]*(z-zf) + w/2.);
    foreach ()
      fan[] *= plane[];
    clear();
    boundary({fan});
    view(tx=-L0/2., ty=-L0/2.);
    cells(alpha = 0, );
    draw_vof("fan");
    save("puck.mp4");
    adapt_wavelet({fan}, (double[]){0.01}, 7, 4);
  }
}

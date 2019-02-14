#if dimension == 3
	#include "SGS.h"
#endif 

#define CP 1005.	// C_p for air 
#define gCONST 9.81	// Gravitational constant
#define TREF 273.	// Kelvin
#define INVERSION .2 	// Kelvin per meter
#define karman 0.4      // von Karman constant 

#define roughY0 0.1     // roughness length 
#define WIND(s) -max((0.15*log(2.*(s-roughY0)+1.)),0.)   // log Wind profile 

#define QFLX (-0.001)			// -20 W/m2
#define BSURF (1.5*b[] - 0.5*b[0,1])   // Estimation of surface b
#define GFLX (-Lambda*(BSURF - bd))
double bd = 0., Lambda = 0.01;         // Grass coupling
#define STRAT(s) gCONST/TREF*(log(30*s + a1 + 1.) - log(a1 + 1.)) + (QFLX/Lambda + bd)
double a1 = 0.;


double crho = 1.;
scalar b[];
scalar * tracers = {b};
	
face vector av[]; 
struct sCase def;

struct sCase {
	double wind;
	double wphi;
};	

void init_physics(){
 	def.wind = 1.;
        def.wphi = 0.;

	b.nodump = false; // TODO

	// redundant if elif function TODO
	if(def.wind == 0.){    	
	    u.n[bottom] = dirichlet(0.);
	    u.t[bottom] = dirichlet(0.);
	    u.n[top] = dirichlet(0.);
 	    u.t[top] = neumann(0.);

            periodic (left);
	
	} else if(fabs(def.wind) > 0.) {
	    u.n[bottom] = dirichlet(0.);
	    u.t[bottom] = dirichlet(0.);
	    u.n[top] = dirichlet(WIND(y));
 	    u.t[top] = neumann(0.);

            periodic (left);
		
	}        
	
	b[bottom] = BSURF;
	b[top] = dirichlet(STRAT(y));

	#if dimension == 3
		u.r[bottom] = dirichlet(0.); 
		u.r[top] = neumann(0.); 
		
		Evis[bottom] = dirichlet(0.); // Flux is explicitly calculated
        	Evis[top] = dirichlet(0.);

		periodic(front);
	#endif  
	foreach() {
		b[] = STRAT(y);
		if(fabs(def.wind) > 0.) {
       	            u.x[] = WIND(y);
		}
	}
}

/* Gravity forcing */
event acceleration(i++){
	foreach_face(y){
		av.y[] = (b[] + b[0,-1])/2.;
	}
}

event inflow(i++){
    double sides = 0.1;
    double relaxtime = dt/10.;
    foreach(){
	if((x < sides*L0 || x > (1-sides)*L0 || 
	    z < sides*L0 || z > (1-sides)*L0   )) {
	    
	    u.x[] = u.x[] + (WIND(y)-u.x[])*relaxtime;
	    u.y[] = u.y[] - u.y[]*relaxtime;
            u.z[] = u.z[] - u.z[]*relaxtime;
 	    b[] = b[] + (STRAT(y) - b[])*relaxtime;
	}
    }
}

mgstats mgb;
//face vector muz;
/* Diffusion */
event tracer_diffusion(i++){
    scalar r[];
    /*
    foreach_face(){
	if(y < Y0 + 1E-10){
            muz.x[] = 0.;
   	} else {
	    muz.x[] = 1.*mu.x[];
        }
    }*/
    
    foreach() {
        r[] = 0;
        if (y < Delta)
            r[] = (QFLX + GFLX)/sq(Delta); // div needed as normalization 
    }
    double flx = 0, bt = 0;
    double fctr = CP*TREF/gCONST;
    foreach_boundary(bottom reduction(+:flx) reduction(+:bt)) {
        flx = flx + (QFLX + GFLX) * sq(Delta);
         bt = bt + BSURF * sq(Delta);
    }
    bt = bt/sq(L0);
    flx = flx/sq(L0);
    fprintf(stderr, "%g %g %g %d\n", t, fctr*flx, fctr*bt/CP, i);  
    
    mgb = diffusion(b, dt, mu, r = r);
}

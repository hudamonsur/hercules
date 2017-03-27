/* finding depth program */

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include "bengalbasin.h"

cvmpayload_t getmantlevalues(double depth_m, double mindepth);
cvmpayload_t getcrustvalues(double depth_m, double mindepth);
cvmpayload_t getbasinvalues(double depth_m, double maxdepth);

int bengal_cvm_query(FILE *fp, double east_m, double north_m, double depth_m, cvmpayload_t* result)
{
    cvmpayload_t temp_result;

	double minlat= 19.9;
	double maxlat= 27;
	double minlong= 86.5;
	double maxlong= 93.4;

	double value;
	double val1;
	double val2;
	double val3;
	double val4;

	double dlat = (maxlat-minlat)/2000;
	double dlong = (maxlong-minlong)/2000;

	long byteval1;
	long byteval2;
	long byteval3;
	long byteval4;

	double inputlat  = minlat  + north_m / (111.01*1000);
	double inputlong = minlong + east_m  / (111.01*1000);

	double diflat  = inputlat  - minlat;
	double diflong = inputlong - minlong;

	if (inputlat > maxlat) inputlat = maxlat;
	if (inputlong > maxlong ) inputlong = maxlong;

	//  if( (inputlat > maxlat) || (inputlat < minlat) || (inputlong < minlong) || (inputlong > maxlong) ) {
	//      printf("The coordinate is out of the region");
	//      return 1;
	//  }

	if( (inputlat < minlat) || (inputlong < minlong) ) {
	    printf("The coordinate is out of the region");
	    return 1;
	}

	long intervalnumberlongitude = diflong/dlong;
	long intervalnumberlatitude = diflat/dlat;

	if ( (intervalnumberlatitude == 0) && (intervalnumberlongitude == 0) ) {

	    byteval1 = intervalnumberlongitude*4 + 4;
	    fseek(fp, byteval1, SEEK_SET);
	    fread(&value, 4, 1, fp);

	} else if ( intervalnumberlatitude == 0 ) {

	    byteval1 = intervalnumberlongitude*4;
	    fseek(fp, byteval1, SEEK_SET);
	    fread(&value, 4, 1, fp);

	} else if ( intervalnumberlongitude == 0 ) {

	    byteval1 = intervalnumberlatitude*8000 + 4;
	    fseek(fp, byteval1, SEEK_SET);
	    fread(&value, 4, 1, fp);

	} else if ( (intervalnumberlatitude == 2000) || (intervalnumberlongitude == 2000) ) {

	    byteval1 = intervalnumberlatitude*8000 + intervalnumberlongitude*4;
	    fseek(fp, byteval1, SEEK_SET);
	    fread(&value, 4, 1, fp);

	} else {

	    byteval1 = intervalnumberlatitude*8000 + intervalnumberlongitude*4;
	    byteval2 = byteval1 + 8000;
	    byteval3 = byteval2 - 4;
	    byteval4 = byteval1 - 4;

	    double lat1 = intervalnumberlatitude*dlat+minlat;
	    double lat2 = lat1 + dlat;

	    double long1 = intervalnumberlongitude*dlong+minlong;
	    double long2 = long1 - dlong;

	    /* seeking values */
	    fseek(fp, byteval1, SEEK_SET);
	    fread(&val1, 4, 1, fp);

	    fseek(fp, byteval2, SEEK_SET);
	    fread(&val2, 4, 1, fp);

	    fseek(fp, byteval3, SEEK_SET);
	    fread(&val3, 4, 1, fp);

	    fseek(fp, byteval4, SEEK_SET);
	    fread(&val4, 4, 1, fp);

	    double R1 = ((long2-east_m)/(long2-long1))*val1 + ((east_m-long1)/(long2-long1))*val2;
	    double R2 = ((long2-east_m)/(long2-long1))*val4 + ((east_m-long1)/(long2-long1))*val3;

	    value = ( (lat2-north_m)/(lat2-lat1) ) * R1 + ( (north_m-lat1)/(lat2-lat1) ) * R2;
	}

	if ( depth_m > ( (value* - 1) + 20000) ) {

	    temp_result = getmantlevalues(depth_m, value);

	} else if ( (depth_m > (value*-1)) && (depth_m <= ((value*-1)+20000)) ) {

	    temp_result = getcrustvalues(depth_m, value);

	} else {

	    temp_result = getbasinvalues(depth_m, value);

	}

    result->Vp  = temp_result.Vp;
    result->Vs  = temp_result.Vs;
    result->rho = temp_result.rho;

	return 0;
}

cvmpayload_t getbasinvalues(double depth_m, double maxdepth){

    cvmpayload_t result;

    double minvs = 500;
    double maxvs = 2000;
    double mindensity = 2400;
    double maxdensity = 2800;

	float vs;
	float density;
	float vp = 0.0;

	float mindepth = 0;

	if ( maxdepth == 0 ) {
		vs = minvs;
		density = mindensity/1000;
	} else if ( (maxdepth > 0) && (maxdepth < 1500) ) {
		vs = minvs
		vp = 2.8 * vs
		density = (float) mindensity

	} else if ((maxdepth > 1500) && (maxdepth < 2000)) {
		vs = 750
		vp = 2.8 * vs
		density = 2550
	} else if ((maxdepth > 2000) && (maxdepth < 2500)) {
		vs = 1100
		vp = 2.8 * vs
		density = 2600

	} else {
		maxdepth = maxdepth*-1;
		vs = (float)( 1100 + ( (maxvs - 1100)*(depth_m - 2500) )/(maxdepth - 2500) );
		vp = 2.8 * vs;
		density = (float)( 2600 + ( (maxdensity - 2600)*(depth_m - 2500) )/(maxdepth - 2500) );
	}
1
	result.Vs = vs;
	result.Vp = vp;
	result.rho = density;

	return result;
}

cvmpayload_t getcrustvalues(double depth_m, double mindepth){

    cvmpayload_t result;

    double minvs = 2700;
    double maxvs = 4200;
    double mindensity = 2900;
    double maxdensity = 3100;

    float vs;
	float density;
	float vp;

	mindepth = mindepth*-1;

	double maxdepth = mindepth+20000;
	vs = (float)(minvs+((maxvs-minvs)*(depth_m-mindepth))/(maxdepth-mindepth));
	vp = 1.8*vs;
	density = (float)(mindensity+((maxdensity-mindensity)*(depth_m-mindepth))/(maxdepth-mindepth));

    result.Vs = vs;
    result.Vp = vp;
    result.rho = density;

    return result;
}

cvmpayload_t getmantlevalues(double depth_m, double mindepth){

    cvmpayload_t result;

    double minvs = 4500;
    double maxvs = 6000;

    float vs;
	float vp;
	float density;

	mindepth = mindepth*-1 + 20000;
	double maxdepth = 40000;

	vs = (float)(minvs+((maxvs-minvs)*(depth_m-mindepth))/(maxdepth-mindepth));
	vp = 1.8*vs;
	density = 3300;

    result.Vs = vs;
    result.Vp = vp;
    result.rho = density;

    return result;
}





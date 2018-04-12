/* finding depth program */

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#include "bengalbasin.h"
#include "constants_bengalbasin.h"

cvmpayload_t getlayervalues(layer_params layer, double depth, double maxdepth, double mindepth);
double * getboreholevalues(double inputlat, double inputlong, double depth);
cvmpayload_t getsurfacevalues(double dlat, double dlong, double diflat, double diflong, double inputlat, double inputlong, double depth);
double getsurfacedepth(double dlat, double dlong, double diflong, double diflat, double east, double north, char* fname);
cvmpayload_t getbhvalues(char* bhname, double depth, double confidence);
double sum_array(double array[], int size);
cvmpayload_t computefinalvalues(cvmpayload_t bh_params, cvmpayload_t surface_params, double bh_confidence);

int bengal_cvm_query(double east_m, double north_m, double depth_m, cvmpayload_t* result)
{
    cvmpayload_t temp_result;
	double dlat = (MAXLAT-MINLAT)/GRIDSTEP;
	double dlong = (MAXLONG-MINLONG)/GRIDSTEP;

	double inputlat = MINLAT + north_m/(DEGREETODIST);
	double inputlong = MINLONG + east_m/(DEGREETODIST);

	double diflat = inputlat-MINLAT;
	double diflong = inputlong-MINLONG;

    if (inputlat > MAXLAT) inputlat = MAXLAT;
    if (inputlong > MAXLONG) inputlong = MAXLONG;

	// if(inputlat>MAXLAT || inputlat<MINLAT || inputlong<MINLONG || inputlong>MAXLONG){
	// 	printf("The co-ordinate is out of the region");
	// 	return 0;
	// }
	// if (DEBUG==1){
 //        puts("<<<<<<<< primary variables loaded>>>>>>>>>>");
	// }

	// choosing among list of boreholes
	// get borehole values and confidence from list of boreholes
	// if (DEBUG==1){
 //        puts("<<<<<<<<<<<<<< calculating borehole values>>>>>>>>>>>>>>>>>>>");
	// };

	double * bh_values;
	cvmpayload_t bh_params;
	//puts("ingetdepth>>>>>>>1");
	bh_values = getboreholevalues(inputlat, inputlong, depth_m);
	double bh_confidence = *(bh_values+0);
	bh_params.Vs = *(bh_values+1);
	bh_params.Vp = *(bh_values+2);
	bh_params.rho = *(bh_values+3);

	// get Vs, Vp and rho from surfaces
	// if (DEBUG==1){
 //        puts("<<<<<<<<<<<<<<<<<< calculating surface values>>>>>>>>>>>>>>>>>>>");
	// };

	//puts("ingetdepth>>>>>>>2");
	cvmpayload_t surface_params = getsurfacevalues(dlat, dlong, diflat, diflong, inputlat, inputlong, depth_m);
 //    if (DEBUG==1){
 //        puts("<<<<<<<<<<<<<< calculating return values>>>>>>>>>>>>>>>>>>>");
	// };
	//temp_result = bh_params;

	//puts("ingetdepth>>>>>>>3");
	temp_result = computefinalvalues(bh_params, surface_params, bh_confidence);
    result->Vp  = temp_result.Vp;
    result->Vs  = temp_result.Vs;
    result->rho = temp_result.rho;

	//puts("ingetdepth>>>>>>>4");
	return 0;
}

cvmpayload_t computefinalvalues(cvmpayload_t bh_params, cvmpayload_t surface_params, double bh_confidence){
    cvmpayload_t result;

    if(bh_params.Vs==0 && bh_params.Vp==0 && bh_params.rho==0){
        result.rho = surface_params.rho;
        result.Vp = surface_params.Vp;
        result.Vs = surface_params.Vs;
    }
    else{
        result.rho = bh_params.rho*bh_confidence + surface_params.rho*(1-bh_confidence);
        result.Vp = bh_params.Vp*bh_confidence + surface_params.Vp*(1-bh_confidence);
        result.Vs = bh_params.Vs*bh_confidence + surface_params.Vs*(1-bh_confidence);
    }

	return result;
}

cvmpayload_t getsurfacevalues(double dlat, double dlong, double diflat, double diflong, double inputlat, double inputlong, double depth){

    cvmpayload_t result;
    result.rho = 0;
    result.Vp = 0;
    result.Vs = 0;

    double values[BIN_COUNT];
    int i = 0;
    double value;
    for(i;i<BIN_COUNT;i++){
        value = getsurfacedepth(dlat, dlong, diflong, diflat, inputlong, inputlat, BIN_NAMES[i]);
        // if(DEBUG==1){printf("%.4f\n", value);}
        values[i] = value;
    }

    if (depth==0){
        if (values[5]==0){
            result = getlayervalues(LAYERS[6], depth, values[6], values[5]);
        }
        else{
            result = getlayervalues(LAYERS[0], depth, values[0], 0);
        }
    }

    int lr_id = 0;
    if(depth>=values[6]){
      result = getlayervalues(LAYERS[7], depth, depth, values[6]);
    }
    else{
        int i = 0;
        for(i;i<BIN_COUNT;i++){
            if(depth<values[i]){
                lr_id = i;
                //printf("%d\n", lr_id);
                //puts("<<<<<<<<< borehole layer determination loop breaks >>>>>>>>");
                break;
            }
        }
        if(lr_id==0){
            result = getlayervalues(LAYERS[0], depth, values[0], 0);
        }
        else if(lr_id==1){
            result = getlayervalues(LAYERS[lr_id],depth,values[lr_id],values[lr_id-1]);
        }
        else{
            int j = lr_id-1;
            int prev_lr_id = 0;
            for(j;j=0;j--){
                if(values[j]>0 && values[j]<values[lr_id]){
                    prev_lr_id = j;
                    break;
                }
            }
            result = getlayervalues(LAYERS[lr_id],depth,values[lr_id],values[prev_lr_id]);
        }
    }

    // if(DEBUG==1){
    //     puts("Surface values:");
    //     printf("%0.4f\n", result.Vs);
    //     printf("%0.4f\n", result.Vp);
    //     printf("%0.4f\n", result.rho);
    // }

    return result;

}

cvmpayload_t getlayervalues(layer_params layer, double depth, double maxdepth, double mindepth){

    cvmpayload_t result;
	double vs;
	double density;
	double vp;

	//maxdepth = maxdepth*-1;
	//mindepth = mindepth*-1;
	//printf("%s %f\n","maxdepth",maxdepth);
	//printf("%s %f\n","mindepth",mindepth);
	if (maxdepth==mindepth){
		vs = layer.minvs;
		vp = layer.vpvsratio*vs;
		density = layer.minrho;
	}
	else{
		vs = (layer.minvs+((layer.maxvs-layer.minvs)*(depth-mindepth))/(maxdepth-mindepth));
		vp = layer.vpvsratio*vs;
		density = (layer.minrho+((layer.maxrho-layer.minrho)*(depth-mindepth))/(maxdepth-mindepth));
	}

	result.Vs=vs;
	result.Vp=vp;
	result.rho=density;
	return result;

}

double getsurfacedepth(double dlat, double dlong, double diflong, double diflat, double east, double north, char* fname){

	double value;
	double val1;
	double val2;
	double val3;
	double val4;
	long byteval1;
	long byteval2;
	long byteval3;
	long byteval4;
//    if (DEBUG==1){
//        puts("<<<<<<<<<<<<<<<<<<< loading binary file>>>>>>>>>>>>>>>>>>>");
//    }
	fp_bengalbasin = fopen(fname, "rb");
//	if (DEBUG==1){
//        puts("<<<<<<<<<<<<<<<<<<<<<<<<<<binary file loaded>>>>>>>>>>>>>>>>>>>>>>>>>");
//
//	}

	if (!fp_bengalbasin)
	{
		printf("Unable to open binary file!");
		return 0;
	}
	else {
		/* bilinear interpolation*/
		long intervalnumberlongitude = diflong/dlong;
		long intervalnumberlatitude = diflat/dlat;

		if (intervalnumberlatitude==0 && intervalnumberlongitude==0){
			byteval1 = intervalnumberlongitude*8 + 8;
			fseek(fp_bengalbasin, byteval1, SEEK_SET);
			fread(&value, 8, 1, fp_bengalbasin);
			// if(DEBUG==1){printf("1>>>>>>>\n");}
		}
		else if (intervalnumberlatitude==0){
			byteval1 = intervalnumberlongitude*8;
			fseek(fp_bengalbasin, byteval1, SEEK_SET);
			fread(&value, 8, 1, fp_bengalbasin);
			// if(DEBUG==1){printf("2>>>>>>>\n");}
		}
		else if(intervalnumberlongitude==0){
			byteval1 = intervalnumberlatitude*16000 + 8;
			fseek(fp_bengalbasin, byteval1, SEEK_SET);
			fread(&value, 8, 1, fp_bengalbasin);
			// if(DEBUG==1){printf("3>>>>>>>\n");}
		}
		else if(intervalnumberlatitude==2000 || intervalnumberlongitude==2000){
			byteval1 = intervalnumberlatitude*16000 + intervalnumberlongitude*8;
			fseek(fp_bengalbasin, byteval1, SEEK_SET);
			fread(&value, 8, 1, fp_bengalbasin);
			// if(DEBUG==1){printf("4>>>>>>>\n");}
		}
		else {
			byteval1 = intervalnumberlatitude*16000 + intervalnumberlongitude*8;
			byteval2 = byteval1 + 16000;
			byteval3 = byteval2 - 8;
			byteval4 = byteval1 - 8;
			double lat1 = intervalnumberlatitude*dlat+MINLAT;
			double lat2 = lat1 + dlat;
			double long1 = intervalnumberlongitude*dlong+MINLONG;
			double long2 = long1 - dlong;

			/* seeking values */
			fseek(fp_bengalbasin, byteval1, SEEK_SET);
			fread(&val1, 8, 1, fp_bengalbasin);
			fseek(fp_bengalbasin, byteval2, SEEK_SET);
			fread(&val2, 8, 1, fp_bengalbasin);
			fseek(fp_bengalbasin, byteval3, SEEK_SET);
			fread(&val3, 8, 1, fp_bengalbasin);
			fseek(fp_bengalbasin, byteval4, SEEK_SET);
			fread(&val4, 8, 1, fp_bengalbasin);

			double R1 = ((long2-east)/(long2-long1))*val1 + ((east-long1)/(long2-long1))*val2;
			//printf("%.4f %.4f\n", long2, east);
			double R2 = ((long2-east)/(long2-long1))*val4 + ((east-long1)/(long2-long1))*val3;
			value = ((lat2-north)/(lat2-lat1))*R1 + ((north-lat1)/(lat2-lat1))*R2;
			//printf("%.4f\n", R2);
			// if(DEBUG==1){printf("5>>>>>>>\n");}
		}
	}
	fclose(fp_bengalbasin);
	//int a;
	//printf("%f >>>>>>> %d\n", value, a);
	//value = value;
	return value;
}


double * getboreholevalues(double inputlat, double inputlong, double depth){

    double bh_confidence_all[100];
    double bh_vs[100];
    double bh_vp[100];
    double bh_rho[100];
    double result[4];
    double bh_confidence = 0;
    double vs = 0;
    double vp = 0;
    double density = 0;
	char ** bh_names = (char**) malloc(4000 * sizeof(char*));
	int bhnumbers = 0;
    int linecount = 0;
	int ch = 0;
//	if (DEBUG==1){
//        puts("<<<<<<<<<<<<<< opening borehole list >>>>>>>>>>>>>>>>>>>");
//	};
	fp_bengalbasin = fopen(BHNAMES_LIST,"r");

	if (!fp_bengalbasin)
	{
		printf("Unable to open borehole list!");
		return 0;
	}
	else {
		while(!feof(fp_bengalbasin)) {
			char str[60];
			if( fgets (str, 60, fp_bengalbasin)!=NULL ) {
            /* writing content to stdout */
                if(str[0]=='\n'){
                    linecount++;
                }
                char *borehole_name = strtok(str, " ");
                char *borehole_lat = strtok(NULL, " ");
                char *borehole_long = strtok(NULL, " ");
//                puts(borehole_name);
//                puts(boreholeinfo[1]);
                double borehole_latf =  atof(borehole_lat);
                double borehole_longf =  atof(borehole_long);
                double borehole_dist = (sqrt(pow((borehole_latf-inputlat),2) + pow((borehole_longf-inputlong),2)))*111.01;
                //double dist_lim = 100;

                if(borehole_dist<BH_INFLUENCE){
                    double borehole_confidence = (BH_INFLUENCE-borehole_dist)/BH_INFLUENCE;
                    char str1[] = ".txt";
                    char * selected_borehole = (char*) malloc(60 * sizeof(char));
                    strcpy(selected_borehole, borehole_name);
                    strcat(selected_borehole, str1);
                    //puts(selected_borehole);
                    cvmpayload_t bh_values = getbhvalues(selected_borehole, depth, borehole_confidence);
                    bh_names[bhnumbers] = selected_borehole;
                    //strcpy(n1[bhnumbers], selected_borehole);
                    bh_confidence_all[bhnumbers] = borehole_confidence;
                    bh_rho[bhnumbers] = bh_values.rho;
                    bh_vp[bhnumbers] = bh_values.Vp;
                    bh_vs[bhnumbers] = bh_values.Vs;
                    //printf("%d %s\n",bhnumbers,bh_names[bhnumbers]);
                    bhnumbers++;
                    free(selected_borehole);
//                    puts(selected_borehole);

                }

            }

		}
	}
	fclose(fp_bengalbasin);
	//printf("%s %d\n", "number of boreholes:", bhnumbers);
	if(bhnumbers>0){
        vs = sum_array(bh_vs,bhnumbers)/sum_array(bh_confidence_all,bhnumbers);
        vp = sum_array(bh_vp,bhnumbers)/sum_array(bh_confidence_all,bhnumbers);
        density = sum_array(bh_rho,bhnumbers)/sum_array(bh_confidence_all,bhnumbers);
        bh_confidence = sum_array(bh_confidence_all,bhnumbers)/bhnumbers;
	}

	// if(DEBUG==1){
 //        puts("borehole values:");
 //        printf("%0.4f\n", vs);
 //        printf("%0.4f\n", vp);
 //        printf("%0.4f\n", density);
 //        printf("%0.4f\n", bh_confidence);
	// }

	result[0] = bh_confidence;
	result[1] = vs;
	result[2] = vp;
	result[3] = density;
	//puts("<<<<<<<<borehole values calculated>>>>>>");

	free(bh_names);
	return result;
}

double sum_array(double array[], int size){
    double sum = 0; // initialize sum

    // Iterate through all elements
    // and add them to sum
    int i = 0;
    for ( i ; i < size; i++)
    sum += array[i];

    return sum;

}

cvmpayload_t getbhvalues(char* bhname, double depth, double confidence){

    cvmpayload_t result;
    cvmpayload_t tmp_result;
    char ** name_layers = (char**) malloc(600 * sizeof(char*));
    double layer_depth[10];
    int layercount = 0;
//    if (DEBUG==1){
//        puts("<<<<<<<<<<<<<< opening borehole file >>>>>>>>>>>>>>>>>>>");
//	};
    fp_borehole = fopen(bhname,"r");
    if(!fp_borehole){
        printf("Borehole file does not exist!");
    }
    else{
        while(!feof(fp_borehole)){
               char str[60];
               if( fgets (str, 60, fp_borehole)!=NULL ){
                    char *str1 = strtok(str, " ");
                    char *depth = strtok(NULL, " ");
                    double lr_depth = atof(depth);
                    char * lr_name = (char*) malloc(60 * sizeof(char));
                    strcpy(lr_name, str1);
                    //printf("%s %f\n", lr_name, lr_depth);
                    name_layers[layercount] = lr_name;
                    layer_depth[layercount] = lr_depth;
                    layercount++;
                    free(lr_name);
               }
        }
    }
    fclose(fp_borehole);
    //printf("%s %d\n", "number of layers in the borehole file:", layercount);
    int i = 1;
    int lr_id=0;
    if(depth>layer_depth[0]){
        tmp_result.rho = 0;
        tmp_result.Vp = 0;
        tmp_result.Vs = 0;
        // if(DEBUG==1){
        //    puts("<<<<<<<<<<<< depth greater than last layer >>>>>>>>>>>>");
        // }
    }
    else{
        for(i;i<layercount;i++){
            if(depth>layer_depth[i]){
                lr_id = i;
                //printf("%d\n", lr_id);
                //puts("<<<<<<<<< borehole layer determination loop breaks >>>>>>>>");
                break;
            }
            //break;
        }
        int j = 0;
        int layer_number=-1;
        //puts("<<>>>>>>>>>><<<<<<<<<>>>>>>>>>>>");
        for(j;j<5;j++){
            //puts(LAYER_NAMES[j]);
            //puts(name_layers[lr_id]);
            int a = strcmp(LAYER_NAMES[j],name_layers[lr_id]);
            if(a==0){
                layer_number = j;
                break;
            }
            //printf("%s %d\n", "Layer number:", layer_number);

        }
        //puts(LAYER_NAMES[layer_number]);
        if(layer_number<0){layer_number = 5;}
        //printf("%s %d %s %f %s %f %s %f\n", "Layer number:", layer_number, "input depth:", depth, "maximum depth:", layer_depth[lr_id-1], "minimum depth:", layer_depth[lr_id]);
        tmp_result = getlayervalues(LAYERS[layer_number], depth, layer_depth[lr_id-1], layer_depth[lr_id]);
    }

    result.rho = confidence*tmp_result.rho;
    result.Vs = confidence*tmp_result.Vs;
    result.Vp = confidence*tmp_result.Vp;
    free(name_layers);
    return result;

}

/* finding depth program */

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#include "bengalbasin.h"
#include "constants_bengalbasin.h"

cvmpayload_t getlayervalues(layer_params layer, double depth, double maxdepth, double mindepth);
cvmpayload_t getlayervalues_sedirock(layer_params layer, double depth, double maxdepth, double mindepth);
// double * getboreholevalues(double inputlat, double inputlong, double depth);
cvmpayload_t getsurfacevalues(double dlat, double dlong, double diflat, double diflong, double inputlat, double inputlong, double depth, double ** surfaces);
double getsurfacedepth(double dlat, double dlong, double diflong, double diflat, double east, double north, double ** surfaces, int i);
// double getsurfacedepth(double dlat, double dlong, double diflong, double diflat, double east, double north, double ** surfaces, int i);
// cvmpayload_t getbhvalues(int id, double depth, double confidence);
// double sum_array(double array[], int size);
// cvmpayload_t computefinalvalues(cvmpayload_t bh_params, cvmpayload_t surface_params, double bh_confidence);
// double getbhconfidence(double bh_dist);
// double getbhdepthconfidence(double max_depth, double depth);
// double weighted_confidence(double array[], int size);
//double avg_array(double array, int size);

int bengal_cvm_query(double east_m, double north_m, double depth_m, cvmpayload_t* result, double ** surfaces)
{
    // cvmpayload_t temp_result;
    double dlat = (MAXLAT-MINLAT)/GRIDSTEP;
    double dlong = (MAXLONG-MINLONG)/GRIDSTEP;

    double inputlat = MINLAT + north_m/(DEGREETODIST); // input coordinates are given in meters and needs to be converted into degree
    double inputlong = MINLONG + east_m/(DEGREETODIST);

    double diflat = inputlat-MINLAT;
    double diflong = inputlong-MINLONG;

    if (inputlat > MAXLAT) inputlat = MAXLAT;
    if (inputlong > MAXLONG) inputlong = MAXLONG;

    if(DB==0){
        result->Vp  = 100;
        result->Vs  = 100;
        result->rho = 100;
    }
    else if(DB==1){
        cvmpayload_t surface_params = getsurfacevalues(dlat, dlong, diflat, diflong, inputlat, inputlong, depth_m, surfaces);
        result->Vp  = surface_params.Vp;
        result->Vs  = surface_params.Vs;
        result->rho = surface_params.rho;
    }
    // if(DB==2){

    //     double * bh_values;
    //     cvmpayload_t bh_params;
    //     bh_values = getboreholevalues(inputlat, inputlong, depth_m);
    //     double bh_confidence = *(bh_values+0);
    //     bh_params.Vs = *(bh_values+1);
    //     bh_params.Vp = *(bh_values+2);
    //     bh_params.rho = *(bh_values+3);

    //     cvmpayload_t surface_params = getsurfacevalues(dlat, dlong, diflat, diflong, inputlat, inputlong, depth_m, surfaces);
    //     temp_result = computefinalvalues(bh_params, surface_params, bh_confidence);

    //     result->Vp  = temp_result.Vp;
    //     result->Vs  = temp_result.Vs;
    //     result->rho = temp_result.rho;
    // }

    return 0;
}

// cvmpayload_t computefinalvalues(cvmpayload_t bh_params, cvmpayload_t surface_params, double bh_confidence){
//     cvmpayload_t result;

//     if(bh_params.Vs==0 && bh_params.Vp==0 && bh_params.rho==0){
//         result.rho = surface_params.rho;
//         result.Vp = surface_params.Vp;
//         result.Vs = surface_params.Vs;
//     }
//     else{
//         result.rho = bh_params.rho*bh_confidence + surface_params.rho*(1-bh_confidence);
//         result.Vp = bh_params.Vp*bh_confidence + surface_params.Vp*(1-bh_confidence);
//         result.Vs = bh_params.Vs*bh_confidence + surface_params.Vs*(1-bh_confidence);
//     }

// 	return result;
// }

cvmpayload_t getsurfacevalues(double dlat, double dlong, double diflat, double diflong, double inputlat, double inputlong, double depth, double ** surfaces){

    cvmpayload_t result;
    double values[BIN_COUNT];
    int i = 0;
    double value;

// Determining depth values of different surfaces provided
    for(;i<BIN_COUNT;i++){
        
        value = getsurfacedepth(dlat, dlong, diflong, diflat, inputlong, inputlat, surfaces, i);
        // value = getsurfacedepth(fpArray[i], inputlat, inputlong, dlat, dlong, diflong, diflat);
//        if(DBUG==1){printf("%.4f\n", value);}
        values[i] = value;
    }

// Determining parameters at the surface in case of sediment and precambrian basement cropped out at the surface
    if (depth==0){
        if (values[5]==0){
            // paramenter values at the surface for precambrian basement
            result = getlayervalues(LAYERS[6], depth, values[6], values[5]);
        }
        else{
            // parameter values at the surface in case of sediment
            result = getlayervalues(LAYERS[0], depth, values[0], 0);
        }
    }
// determining parameter values for all the other surfaces
    int lr_id = 0;
    if(depth>=values[6]){
        // determining parameter values for material below moho depth
        result = getlayervalues(LAYERS[7], depth, depth, values[6]);
    }
    else{
        int i = 0;
        for(;i<BIN_COUNT;i++){
            if(depth<values[i]){
                lr_id = i;
                // printf("%d\n", lr_id);
                //puts("<<<<<<<<< borehole layer determination loop breaks >>>>>>>>");
                break;
            }
        }
        if(lr_id==0){
            // determining parameter values for upper sedimentary layer
            result = getlayervalues(LAYERS[0], depth, values[0], 0);
        }
        else if(lr_id==1){
            result = getlayervalues(LAYERS[lr_id],depth,values[lr_id],values[lr_id-1]);
        }
        else{
            int j = lr_id-1;
            int prev_lr_id = 0;
            for(;j=0;j--){
                if(values[j]>0 && values[j]<values[lr_id]){
                    prev_lr_id = j;
                    break;
                }
            }
            // if(lr_id==6){
            //     result = getlayervalues(LAYERS[lr_id],depth,values[lr_id],values[prev_lr_id]);
            // }
            // else{
            //     result = getlayervalues_sedirock(LAYERS[lr_id],depth,values[lr_id],values[prev_lr_id]);
            // }
            result = getlayervalues(LAYERS[lr_id],depth,values[lr_id],values[prev_lr_id]);
        }
    }

    if(DBUG==1){
        puts("Surface values:");
        printf("%0.4f\n", result.Vs);
        printf("%0.4f\n", result.Vp);
        printf("%0.4f\n", result.rho);
    }

    return result;

}

cvmpayload_t getlayervalues(layer_params layer, double depth, double maxdepth, double mindepth){

    cvmpayload_t result;
    double vs;
    double density;
    double vp;
    // puts("<<<<<<<<<<< values from fixed values >>>>>>>>");

    if (maxdepth==mindepth){
        vs = layer.minvs/1000;
        // vp = layer.vpvsratio*vs;
        // density = layer.minrho;
    }
    else{
        vs = (layer.minvs+((layer.maxvs-layer.minvs)*(depth-mindepth))/(maxdepth-mindepth))/1000;
        // vp = layer.vpvsratio*vs;
        // density = (layer.minrho+((layer.maxrho-layer.minrho)*(depth-mindepth))/(maxdepth-mindepth));
    }

    vp = 0.9409 + 2.0947*vs - pow(vs,2)*0.8206 + pow(vs,3)*0.2683 - pow(vs,4)*0.0251;
    if (vs<=0.3){
        density = 1 + ((1.53*(pow(vs,0.85)))/(0.35+(1.889*(pow(vs,1.7)))));
    }
    else if(vs>0.3 && vs<=3.55){
        density = 1.74*(pow(vp,0.25));
    }
    else{
        density = 1.6612*vp - 0.4721*(pow(vp,2)) + 0.0671*(pow(vp,3)) - 0.0043*(pow(vp,4)) + 0.000106*(pow(vp,5));
    }

    result.Vs=vs*1000;
    result.Vp=vp*1000;
    result.rho=density*1000;
    return result;

}

// cvmpayload_t getlayervalues_sedirock(layer_params layer, double depth, double maxdepth, double mindepth){

//     cvmpayload_t result;
//     double vs;
//     double density;
//     double vp;
//     double vpft;
//     double age;
//     double expo = 0.1667;
//     // puts("<<<<<<<<<<< values from faust's equation >>>>>>>>");

//     if (maxdepth==mindepth){
//         age = layer.minage*1000000;
//         vpft = layer.constantk*(pow(depth*3.28*age,expo));
//         vp = vpft/(3.28*1000);
//         // printf("%0.4f\n", vp);

//     }
//     else{
//         age = (layer.minage+((layer.maxage-layer.minage)*(depth-mindepth))/(maxdepth-mindepth))*1000000;
//         vpft = layer.constantk*(pow(depth*3.28*age,expo));
//         vp = vpft/(3.28*1000);
        
//     }
//     vs = 0.7858 - 1.2344*vp + 0.7949*(pow(vp,2)) - 0.1238*(pow(vp,3)) + 0.0064*(pow(vp,4));
//     // printf("%0.4f\n", vs);
//     density = 1.6612*vp - 0.4721*(pow(vp,2)) + 0.0671*(pow(vp,3)) - 0.0043*(pow(vp,4)) + 0.000106*(pow(vp,5));

//     result.Vs=vs*1000;
//     result.Vp=vp*1000;
//     result.rho=density*1000;
//     return result;

// }

double getsurfacedepth(double dlat, double dlong, double diflong, double diflat, double east, double north, double ** surfaces, int i){

 double value;
 double val1;
 double val2;
 double val3;
 double val4;
 long index;
 long index1;
 long index2;
 long index3;
 long index4;

    /* bilinear interpolation*/
    long intervalnumberlongitude = diflong/dlong;
    //printf("%d %d\n", intervalnumberlongitude, i);
    long intervalnumberlatitude = diflat/dlat;
    //printf("%d %d\n", intervalnumberlatitude, i);

    if (intervalnumberlatitude==0 && intervalnumberlongitude==0){
        index = intervalnumberlongitude;
        value = surfaces[i][index];
        if(DBUG==1){printf("%f 1>>>>>>>\n", value);};
    }
    else if (intervalnumberlatitude==0){
        index = intervalnumberlongitude;
        value = surfaces[i][index];
        if(DBUG==1){printf("2>>>>>>>\n");}
    }
    else if(intervalnumberlongitude==0){
        index = intervalnumberlatitude*2000;
        value = surfaces[i][index];
        if(DBUG==1){printf("3>>>>>>>\n");}
    }
    else if(intervalnumberlatitude==2000 || intervalnumberlongitude==2000){
        index = intervalnumberlatitude*2000 + intervalnumberlongitude;
        value = surfaces[i][index];
        if(DBUG==1){printf("4>>>>>>>\n");}
    }
    else {
        index1 = intervalnumberlatitude*2000 + intervalnumberlongitude;
        val1 = surfaces[i][index1];
        index2 = index1 + 2000;
        val2 = surfaces[i][index2];
        index3 = index2 - 1;
        val3 = surfaces[i][index3];
        index4 = index1 - 1;
        val4 = surfaces[i][index4];
        double lat1 = intervalnumberlatitude*dlat+MINLAT;
        double lat2 = lat1 + dlat;
        double long1 = intervalnumberlongitude*dlong+MINLONG;
        double long2 = long1 - dlong;

        double R1 = ((long2-east)/(long2-long1))*val1 + ((east-long1)/(long2-long1))*val2;
        //printf("%.4f %.4f\n", long2, east);
        double R2 = ((long2-east)/(long2-long1))*val4 + ((east-long1)/(long2-long1))*val3;
        value = ((lat2-north)/(lat2-lat1))*R1 + ((north-lat1)/(lat2-lat1))*R2;
    }

 //printf("%f >>>>>>> %d\n", value, a);
 //value = value;
 return value;
}

// double getsurfacedepth(FILE *fp, double inputlat, double inputlong, double dlat, double dlong, double diflong, double diflat){

//     double value;
//     double val1;
//     double val2;
//     double val3;
//     double val4;
//     long byteval1;
//     long byteval2;
//     long byteval3;
//     long byteval4;

//     if (!fp)
//     {
//         printf("Unable to open file!");
//         return 0;
//     }

//     else {
//         /* bilinear interpolation*/
//         long intervalnumberlongitude = diflong/dlong;
//         long intervalnumberlatitude = diflat/dlat;

//         if (intervalnumberlatitude==0 && intervalnumberlongitude==0){
//             byteval1 = intervalnumberlongitude*8 + 8;
//             fseek(fp, byteval1, SEEK_SET);
//             fread(&value, 8, 1, fp);
//         }

//         else if (intervalnumberlatitude==0){
//             byteval1 = intervalnumberlongitude*8;
//             fseek(fp, byteval1, SEEK_SET);
//             fread(&value, 8, 1, fp);
//         }

//         else if(intervalnumberlongitude==0){
//             byteval1 = intervalnumberlatitude*16000 + 8;
//             fseek(fp, byteval1, SEEK_SET);
//             fread(&value, 8, 1, fp);
//         }

//         else if(intervalnumberlatitude==2000 || intervalnumberlongitude==2000){
//             byteval1 = intervalnumberlatitude*16000 + intervalnumberlongitude*8;
//             fseek(fp, byteval1, SEEK_SET);
//             fread(&value, 8, 1, fp);
//         }

//         else {
//             byteval1 = intervalnumberlatitude*16000 + intervalnumberlongitude*8;
//             byteval2 = byteval1 + 16000;
//             byteval3 = byteval2 - 8;
//             byteval4 = byteval1 - 8;
//             double lat1 = intervalnumberlatitude*dlat+MINLAT;
//             double lat2 = lat1 + dlat;
//             double long1 = intervalnumberlongitude*dlong+MINLONG;
//             double long2 = long1 - dlong;

//             /* seeking values */
//             fseek(fp, byteval1, SEEK_SET);
//             fread(&val1, 8, 1, fp);
//             fseek(fp, byteval2, SEEK_SET);
//             fread(&val2, 8, 1, fp);
//             fseek(fp, byteval3, SEEK_SET);
//             fread(&val3, 8, 1, fp);
//             fseek(fp, byteval4, SEEK_SET);
//             fread(&val4, 8, 1, fp);


//             double R1 = ((long2-inputlong)/(long2-long1))*val1 + ((inputlong-long1)/(long2-long1))*val2;
//             double R2 = ((long2-inputlong)/(long2-long1))*val4 + ((inputlong-long1)/(long2-long1))*val3;

//             value = ((lat2-inputlat)/(lat2-lat1))*R1 + ((inputlat-lat1)/(lat2-lat1))*R2;
//         }
//     }
//     fseek(fp, 0, SEEK_SET); // to reset the pointer to the start of the file.
//     return value;
// }

// double * getboreholevalues(double inputlat, double inputlong, double depth){

//     double bh_confidence_all[100];
//     double bh_vs[100];
//     double bh_vp[100];
//     double bh_rho[100];
//     double result[4];
//     double bh_confidence = 0;
//     double vs = 0;
//     double vp = 0;
//     double density = 0;
//     int bhnumbers = 0;

//     int i;
//     for(i=0;i<55;i++){

//         char * borehole_name = (char*) malloc(60 * sizeof(char));
//         strcpy(borehole_name,BOREHOLE_INFO[i].name);
//         double borehole_latf =  BOREHOLE_INFO[i].lattitude;
//         double borehole_longf =  BOREHOLE_INFO[i].longitude;
//         double borehole_dist = (sqrt(pow((borehole_latf-inputlat),2) + pow((borehole_longf-inputlong),2)))*111.01;
//                 //double dist_lim = 100;
//         if(borehole_dist<BH_INFLUENCE){
//             double borehole_confidence = getbhconfidence(borehole_dist);
//             cvmpayload_t bh_values = getbhvalues(i, depth, borehole_confidence);
//             bh_confidence_all[bhnumbers] = borehole_confidence;
//             bh_rho[bhnumbers] = bh_values.rho;
//             bh_vp[bhnumbers] = bh_values.Vp;
//             bh_vs[bhnumbers] = bh_values.Vs;
//             bhnumbers++;
//         }
//         free(borehole_name);
//     }

//     if(bhnumbers>0){
//         vs = sum_array(bh_vs,bhnumbers)/sum_array(bh_confidence_all,bhnumbers);
//         vp = sum_array(bh_vp,bhnumbers)/sum_array(bh_confidence_all,bhnumbers);
//         density = sum_array(bh_rho,bhnumbers)/sum_array(bh_confidence_all,bhnumbers);
//         bh_confidence = weighted_confidence(bh_confidence_all,bhnumbers);
//     }

//     // if(DBUG==1){
//     //     puts("borehole values:");
//     //     printf("%0.4f\n", vs);
//     //     printf("%0.4f\n", vp);
//     //     printf("%0.4f\n", density);
//     //     printf("%0.4f\n", bh_confidence);
//     // }

//     result[0] = bh_confidence;
//     result[1] = vs;
//     result[2] = vp;
//     result[3] = density;
//     //puts("<<<<<<<<borehole values calculated>>>>>>");

// //  free(bh_names);
//     return result;
// }

// double getbhconfidence(double bh_dist){

//     double value = (pow((bh_dist/(BH_INFLUENCE/2)),3))*-1;
//     double bh_confidence = exp(value);
//     return bh_confidence;

// }

// double weighted_confidence(double array[], int size){
//     double confidence = 0;
//     int i = 0;
//     double conf[size];
//     for(;i<size;i++){
//         conf[i] = array[i]*array[i];
//     }
//     confidence = sum_array(conf,size)/sum_array(array,size);
//     return confidence;
// }

// double sum_array(double array[], int size){
//     double sum = 0; // initialize sum

//     // Iterate through all elements
//     // and add them to sum
//     int i = 0;
//     for (;i<size;i++){
//         sum += array[i];
//     }

//     return sum;

// }

// double getbhdepthconfidence(double max_depth, double depth){

//     double value = (pow((depth/(max_depth/2)),3))*-1;
//     double bh_confidence = exp(value);
//     return bh_confidence;

// }

// cvmpayload_t getbhvalues(int id, double depth, double confidence){

//     cvmpayload_t result;
//     cvmpayload_t tmp_result;
//     int layer_id[15];
// //    char ** name_layers = (char**) malloc(600 * sizeof(char*));
//     double layer_depth[15];
//     int layercount = 0;
// //    if (DBUG==1){
// //        puts("<<<<<<<<<<<<<< opening borehole file >>>>>>>>>>>>>>>>>>>");
// //  };
// //    puts(BOREHOLE_INFO[id].name);
//     rewind(BOREHOLE_INFO[id].bh);
//     fp_borehole = BOREHOLE_INFO[id].bh;
//     if(!fp_borehole){
//         printf("Borehole file does not exist!");
//     }
//     else{
//         while(!feof(fp_borehole)){
//                char str[60];
//                if( fgets (str, 60, fp_borehole)!=NULL ){
//                     char *str1 = strtok(str, " ");
//                     char *depth = strtok(NULL, " ");
//                     double lr_depth = atof(depth);
//                     int lr_id = atoi(str1);
// //                    char * lr_name = (char*) malloc(60 * sizeof(char));
// //                    strcpy(lr_name, str1);
// //                    printf("%d %f\n", lr_id, lr_depth);
// //                    name_layers[layercount] = lr_name;
//                     layer_id[layercount] = lr_id;
//                     layer_depth[layercount] = lr_depth;
//                     layercount++;
// //                    free(lr_name);
//                }
//         }
//     }
// //    printf("%s %d\n", "number of layers in the borehole file:", layercount);
//     double depthconfidence = getbhdepthconfidence(layer_depth[0], depth);
//     if(depth==0){
//         tmp_result = getlayervalues(LAYERS[0], depth, layer_depth[layercount-1], 0);
//     }
//     else{
//         int i = 1;
//         int lr_id=0;
//         if(depth>layer_depth[0]){
//             tmp_result.rho = 0;
//             tmp_result.Vp = 0;
//             tmp_result.Vs = 0;
//         }
//         else{
//             for(;i<layercount;i++){
//                 if(depth>layer_depth[i]){
//                     lr_id = i;
// //                    printf("layer id: %d\n", lr_id);
//                     //puts("<<<<<<<<< borehole layer determination loop breaks >>>>>>>>");
//                     break;
//                 }
//                 //break;
//             }
//             int j = 0;
//             int layer_number=-1;
//             //puts("<<>>>>>>>>>><<<<<<<<<>>>>>>>>>>>");
//             for(;j<BIN_COUNT;j++){
// //                puts(LAYER_NAMES[j]);
// //                puts(name_layers[lr_id]);
// //                int a = strcasecmp(LAYER_NAMES[j],name_layers[lr_id]);
//                 if(LAYER_NAMES[j]==layer_id[lr_id]){
//                     layer_number = j;
//                     break;
//                 }
// //                printf("%s %d\n", "Layer number:", layer_number);

//             }
// //            puts(LAYER_NAMES[layer_number]);
// //            if(layer_number<0){layer_number = 5;}
// //            printf("%s %d %s %f %s %f %s %f %s %f\n", "Layer number:", layer_number, "input depth:", depth, "maximum depth:", layer_depth[lr_id-1], "minimum depth:", layer_depth[lr_id], "Confidence", confidence);
//             // if(DBUG==1){
//             //    printf("%s %d %s %f %s %f %s %f %s %f\n", "Layer number:", layer_number, "input depth:", depth, "maximum depth:", layer_depth[lr_id-1], "minimum depth:", layer_depth[lr_id], "Confidence", confidence);
//             // }
//             tmp_result = getlayervalues(LAYERS[layer_number], depth, layer_depth[lr_id-1], layer_depth[lr_id]);
//         }

//     }

//     result.rho = confidence*tmp_result.rho;
//     result.Vs = confidence*tmp_result.Vs;
//     result.Vp = confidence*tmp_result.Vp;
// //    free(name_layers);
//     return result;
// }

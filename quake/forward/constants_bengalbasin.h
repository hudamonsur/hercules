#ifndef CONSTANTS_BENGALBASIN_H_
#define CONSTANTS_BENGALBASIN_H_
#include<stdio.h>
//#include "filenames.h"

double MINLAT = 19.9; // minimum latitude of the extent of simulation region in degree decimal
double MAXLAT = 27; // maximum latitude of the extent of simulation region in degree decimal
double MINLONG = 86.5; // minimum longitude of the extent of simulation region in degree decimal
double MAXLONG = 93.4; // maximum longitude of the extent of simulation region in degree decimal

int GRIDSTEP = 2000; // grid step of the binary files generated from the contours
double DEGREETODIST = 111.01*1000; // degree to distance multiplier in meters

//double SEDIMENTDEPTH = 100; // fixed sediment depth assumed for the model in meters
double APPROXCRUSTTHICKNESS = 20000; // average approximated crustal thickness
int BH_INFLUENCE = 20; // borehole influence distance.
char BHNAMES_LIST[] = "borehole_list.txt";

// typedef struct eq_constants
// {
//     double c1, c2, c3, c4, c5;   
// } eq_constants;

// eq_constants VPTOROH; // equation for converting P-Wave velocity to density accodring to Nafe and Drake (1961)
// eq_constants VPTOVS; // equation for converting P-wave velocity to S-wave velocity according to Brocher (2005)
// eq_constants VSTOVP; // equation for converting S-wave velocity to P-wave velocity according to Brocher (2005)

// void initiate_equations(){
//     VPTOROH.c1 = 1.6612;
//     VPTOROH.c2 = 0.4721;
//     VPTOROH.c3 = 0.0671;
//     VPTOROH.c4 = 0.0043;
//     VPTOROH.c5 = 0.000106;

//     VPTOVS.c1 = 0.7858; 
//     VPTOVS.c2 = 1.2344;
//     VPTOVS.c3 = 0.7949;
//     VPTOVS.c4 = 0.1238;
//     VPTOVS.c5 = 0.0064;

//     VSTOVP.c1 = 0.9409;
//     VSTOVP.c2 = 2.0947;
//     VSTOVP.c3 = 0.8206;
//     VSTOVP.c4 = 0.2683;
//     VSTOVP.c5 = 0.0251;
// }

typedef struct layer_params {
    double minvs, maxvs, minrho, maxrho, vpvsratio, minage, maxage, constantk;
} layer_params;

typedef struct boreholes {
    char * name;
    double lattitude;
    double longitude;
    FILE * bh;
} boreholes;

int LAYER_NAMES[] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
};

const char * BIN_NAMES[] = {
    "depth_sediment.bin",
    "depth_dupitila.bin",
    "depth_tipam.bin",
    "depth_bokabil.bin",
    "depth_bhuban.bin",
    "depth_precambrian.bin",
    "depth_moho.bin",
};
int BIN_COUNT = 7;
int LAYER_COUNT = 8;
layer_params LAYERS[8];
double LAYER_MINVS[] = {100, 700, 1000, 1300, 1700, 2200, 3300, 4500};
double LAYER_MAXVS[] = {500, 1000, 1300, 1700, 2200, 2400, 3900, 4500};
double LAYER_MINRHO[] = {2000, 2100, 2200, 2350, 2450, 2550, 2670, 3300};
double LAYER_MAXRHO[] = {2050, 2200, 2350, 2450, 2550, 2600, 2800, 3300};
double LAYER_VPVSRATIO[] = {2.2, 2.1, 2.1, 2.0, 2.0, 1.9, 1.8, 1.7};
double LAYER_MINAGE[] = {0, 1.5, 5.5, 11, 18, 24, 130, 300};
double LAYER_MAXAGE[] = {1.5, 5.5, 11, 18, 24, 130, 300, 2000};
double LAYER_CONSTANT[] = {150, 150, 150, 150, 150, 150, 150, 170};

void initiate_layers(){
    int i = 0;
    for(i=0;i<LAYER_COUNT;i++){
         LAYERS[i].minvs = LAYER_MINVS[i];
         LAYERS[i].maxvs = LAYER_MAXVS[i];
         LAYERS[i].minrho = LAYER_MINRHO[i];
         LAYERS[i].maxrho = LAYER_MAXRHO[i];
         LAYERS[i].vpvsratio = LAYER_VPVSRATIO[i];
         LAYERS[i].minage = LAYER_MINAGE[i];
         LAYERS[i].maxage = LAYER_MAXAGE[i];
         LAYERS[i].constantk = LAYER_CONSTANT[i];
    }
}

boreholes BOREHOLE_INFO[55];
void initiate_boreholes(){
    FILE * fp_bengalbasin;
    fp_bengalbasin = fopen(BHNAMES_LIST,"r");
    int k=0;
    if (!fp_bengalbasin)
    {
        printf("Unable to open borehole list!");
    }
    else {
        while(!feof(fp_bengalbasin)) {
            char str[60];
            if( fgets (str, 60, fp_bengalbasin)!=NULL ) {
            /* writing content to stdout */
//                if(str[0]=='\n'){
//                    linecount++;
//                }
                char *borehole_name = strtok(str, " ");
                char *borehole_lat = strtok(NULL, " ");
                char *borehole_long = strtok(NULL, " ");
//                puts(borehole_name);
//                puts(boreholeinfo[1]);
                double borehole_latf =  atof(borehole_lat);
                double borehole_longf =  atof(borehole_long);

                BOREHOLE_INFO[k].name = (char*) malloc(60 * sizeof(char));
                strcpy(BOREHOLE_INFO[k].name, borehole_name);
                BOREHOLE_INFO[k].lattitude = borehole_latf;
                BOREHOLE_INFO[k].longitude = borehole_longf;

                char str1[] = ".txt";
                strcat(borehole_name, str1);
                BOREHOLE_INFO[k].bh = fopen(borehole_name,"r");
                k++;
            }
        }
    }
    fclose(fp_bengalbasin);

}
void initiate_globals(){
    initiate_layers();
    // initiate_boreholes();
    // initiate_equations();


}

//LAYERS[0].minvs = 100; // minimum shear wave velocity of the quaternary sediment in meter/sec
//LAYERS[0].maxvs = 500; // maximum shear wave velocity of the quaternary sediment in meter/sec
//LAYERS[0].minrho = 2000; // minimum density of the quaternary sediment in kg/m3
//LAYERS[0].maxrho = 2050; // maximum density of the quaternary sediment in kg/m3
//LAYERS[0].vpvsratio = 2.2; // ratio between P-wave and S-wave velocity Dupitila deposit
//
//LAYERS[1].minvs = 750; // minimum shear wave velocity of the Dupitila deposit in meter/sec
//LAYERS[1].maxvs = 1000; // maximum shear wave velocity of the Dupitila deposit in meter/sec
//LAYERS[1].minrho = 2100; // minimum density of the Dupitila deposit in kg/m3
//LAYERS[1].maxrho = 2200; // maximum density of the Dupitila deposit in kg/m3
//LAYERS[1].vpvsratio = 2.1; // ratio between P-wave and S-wave velocity of the Dupitila deposit
//
//LAYERS[2].minvs = 1050; // minimum shear wave velocity of the Tipam deposit in meter/sec
//LAYERS[2].maxvs = 1100; // maximum shear wave velocity of the Tipam deposit in meter/sec
//LAYERS[2].minrho = 2200; // minimum density of the Tipam deposit in kg/m3
//LAYERS[2].maxrho = 2350; // maximum density of the Tipam deposit in kg/m3
//LAYERS[2].vpvsratio = 2.1; // ratio between P-wave and S-wave velocity of the Tipam deposit
//
//LAYERS[3].minvs = 1250; // minimum shear wave velocity of the Bokabil deposit in meter/sec
//LAYERS[3].maxvs = 1650; // maximum shear wave velocity of the Bokabil deposit in meter/sec
//LAYERS[3].minrho = 2350; // minimum density of the Bokabil deposit in kg/m3
//LAYERS[3].maxrho = 2450; // maximum density of the Bokabil deposit in kg/m3
//LAYERS[3].vpvsratio = 2.0; // ratio between P-wave and S-wave velocity of the Bokabil deposit
//
//LAYERS[4].minvs = 1700; // minimum shear wave velocity of the Bhuban deposit in meter/sec
//LAYERS[4].maxvs = 2200; // maximum shear wave velocity of the Bhuban deposit in meter/sec
//LAYERS[4].minrho = 2450; // minimum density of the Bhuban deposit in kg/m3
//LAYERS[4].maxrho = 2550; // maximum density of the Bhuban deposit in kg/m3
//LAYERS[4].vpvsratio = 2.0; // ratio between P-wave and S-wave velocity of the Bhuban deposit
//
//LAYERS[5].minvs = 2250; // minimum shear wave velocity of the PreMiocene deposit in meter/sec
//LAYERS[5].maxvs = 2400; // maximum shear wave velocity of the PreMiocene deposit in meter/sec
//LAYERS[5].minrho = 2550; // minimum density of the PreMiocene deposit in kg/m3
//LAYERS[5].maxrho = 2600; // maximum density of the PreMiocene deposit in kg/m3
//LAYERS[5].vpvsratio = 1.9; // ratio between P-wave and S-wave velocity of the PreMiocene deposit
//
//LAYERS[6].minvs = 3250; // minimum shear wave velocity of the PreCambrian deposit in meter/sec
//LAYERS[6].maxvs = 3500; // maximum shear wave velocity of the PreCambrian deposit in meter/sec
//LAYERS[6].minrho = 2670; // minimum density of the PreCambrian deposit in kg/m3
//LAYERS[6].maxrho = 2800; // maximum density of the PreCambrian deposit in kg/m3
//LAYERS[6].vpvsratio = 1.8; // ratio between P-wave and S-wave velocity of the PreCambrian deposit
//
//LAYERS[7].minvs = 4500; // minimum shear wave velocity of the Mantle in meter/sec
//LAYERS[7].maxvs = 6000; // maximum shear wave velocity of the Mantle in meter/sec
//LAYERS[7].minrho = 3300; // density of the Mantle in kg/m3
//LAYERS[7].maxrho = 3300;
//LAYERS[7].vpvsratio = 1.7; // ratio between P-wave and S-wave velocity of the Mantle
FILE* fp_bengalbasin;
FILE* fp_borehole;
// int DEBUG = 0;
int DB = 1;

//typedef struct loc {
//    double laty, longx;
//} loc;
//
//loc * MINLOC = (loc*) malloc(2 * sizeof(loc));
//MINLOC.laty = MINLAT;
//MINLOC.longx = MINLONG;
//
//loc * loc_dif(loc * p1, loc * p2){
//    loc * p3 = (loc*) malloc(2 * sizeof(loc));
//    p3.laty = p1.laty - p2.laty;
//    p3.longx = p1.longx - p2.longx;
//    return p3;
//
//}

#endif // CONSTANTS_BENGALBASIN_H_

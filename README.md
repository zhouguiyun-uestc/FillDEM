# FillDEM
**Manuscript Title**: An efficient variant of the Priority-Flood algorithm for filling depressions in raster digital elevation models

**Authors**: Guiyun Zhou, Zhongxuan Sun, Suhua Fu

**Corresponding author**: Guiyun Zhou (zhouguiyun@uestc.edu.cn)

This repository contains the source codes of the algorithms presented in the manuscript above. These codes were used in performing the tests described in the manuscript.

The coded can be compiled on both Windows and Linux platforms. 

FillDEM supports floating-point GeoTIFF file format through the GDAL library. Please include GDAL library into your compilation. GDAL 1.91 was used in our experiments.

The algorithms described in the manuscript can be run using the following command line:

FillDEM AlgorithmName Input_DEM

**Example**: FillDEM zhou-onepass dem.tif.  Use the one-pass implementation  proposed in the manuscript to fill the input DEM "dem.tif" and create a filled DEM "dem_filled_zhou-onepass.tif".

The algorithms available are described briefly below and in greater detail in the manuscript.

**wang**: Use the variant in Wang and Liu (2006) to fill the DEM. The name of the filled dem is ended with "_filled_wang". 

**barnes**: Use the variant in Barnes et al. (2014) to fill the DEM.  The name of the filled dem is ended with "_filled_barnes".

**zhou-twopass**: Use the two-pass implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-twopass".

**zhou-onepass**: Use the one-pass implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-onepass".

**zhou-direct**: Use the direct implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-direct".

<p> The test data used in the manuscript can be downloaded at http://www.mngeo.state.mn.us/. You need ArcGIS to convert the DEM into GeoTIFF format.

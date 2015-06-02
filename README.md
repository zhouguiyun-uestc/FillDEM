# FillDEM
**Manuscript Title**: An efficient variant of the Priority-Flood algorithm for filling depressions in raster digital elevation models

**Authors**: Guiyun Zhou, Zhongxuan Sun, Suhua Fu

**Corresponding author**: Guiyun Zhou (zhouguiyun@uestc.edu.cn)

This repository contains the implementation of the algorithms presented in the manuscript above. These implementations were used in performing the tests described in the manuscript.

There is source code for the five pseudocode variants of the Priority-Flood algorithms presented in the manuscript. All the code can be compiled using Visual Studio 2010. The result is a program called FillDEM.exe. The tool can be run under 64-bit Microsoft Windows.

FillDEM supports floating-point GeoTIFF file format through the GDAL library.

The algorithms described in the manuscript can be run using the following command line:

FillDEM AlgorithmName Input_DEM

**Example**: FillDEM zhou-onepass dem.tif.  Use the one-pass implementation  proposed in the manuscript to fill the input DEM "dem.tif" and create a filled DEM "dem_filled_zhou-onepass.tif".

The algorithms available are described briefly below and in greater detail in the manuscript.

**wang**: Use the variant in Wang and Liu (2006) to fill the DEM. The name of the filled dem is ended with "_filled_wang". 

**barnes**: Use the variant in Barnes et al. (2014) to fill the DEM.  The name of the filled dem is ended with "_filled_barnes".

**zhou-twopass**: Use the two-pass implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-twopass".

**zhou-onepass**: Use the one-pass implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-onepass".

**zhou-direct**: Use the direct implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-direct".

<p>The <i>binary64</i> directory contains the executable files for the 64bit Windows. The <i>src</i> folder contains the solution project of the Visual Studio 2010.

<p> The test data used in the manuscript can be downloaded at http://www.mngeo.state.mn.us/. You need ArcGIS to convert the DEM into GeoTIFF format.

# Spectrum View

By Joaquin Reyes

## **About spectrumview**

Spectrum View is a C++ program to build spatially resolved spectroscopy measurements, the program was created to be used with Electron Energy Loss Spectroscopy experiments. A variety of techniques can take advantage of what this program offers, as the EELS results can be similar to SNOM, Raman Microscopy, IR Microscopy, etc. Instruments used in this techniques can come with software to perform spectrum mapping experiments, however, physical limitations of the probe and instrument optics can affect the accuracy in the results and for this scenarios the best experimental approach is to do a point by point acquisition of the spectra and then create the map with image processing.

Spectrum view is an image processing tool to create spectrum maps out of point by point spectra. The way this is done can be separated in 3 steps:

1. *Opening the files:* All the acquired point spectra must be saved in a single directory following the naming guidelines that are explained later in this document. The program uses `std::filesystem` functions to open the directory, read the file name to extract the point coordinates of the spectrum and read the file content to extract the two axis (i.e. energy and intensity) contained by a spectrum. By using the `class spectrum` from the header file "spectrum_map.hpp" it creates an object of each file of the directory with both the spectral and spatial information.

2. *Creating the map:* With the `class spectrum` object information, the program creates a `std::map` with a tuple containing the coordinates as the map key and the intensity (integrated or interpolated, see the **Using spectrumview** section for more information) at a selected energy associated to each map key; it also creates a `std::set` of tuples with all the existing coordinates from the file. This two created containers are then used as input parameters to create a `class data_map` object from "spectrum_map.hpp"; the data_map contains a flattened 2D matrix to create the image:

   The dimensions of this matrix are defined by the number of possible values for x (for the columns) and y (for the rows) coordinates that were read from the file names. Each element within the matrix corresponds to a point in space where data may or may not have been acquired. If there is data at the corresponding point, the value of the element will be equal to the intensity stored in the `std::map` for that point, if there is no data for the point, the element equals zero.

   The `class data_map` stores the axis positions and step size in between positions for x and y separately, and the dimensions of the matrix.

3. *Creating the output file:* This program can create two kinds of files: '.txt' files that can later be processed in other languages or software and BMP files showing the spatial distribution of the intensity at a selected energy.
   * *The '.txt' files* can be for the raw matrix or a BMP formatted matrix.
     * If the raw matrix is requested 3 files will be created: one with the 2D matrix with fixed width columns, and two with the x and y axis values to properly define the dimensions. The reason of this, is that in some experiments the step size can be uneven and if there is no information about the size scale the map will not be spatially accurate even if it contains the proper information.
     * If the formatted matrix is requested, the program creates a 2D matrix with even step size (pixel size) and with dimensions that have the appropriate values to be turned into a bitmap (the row and column dimensions are multiples of 4).* To fill the formatted 2D matrix with the information coming from the intensity of the raw map, the real dimensions are considered, every pixel has the intensity value from the lower position between the closest raw map position values for a given pixel.  
  The output '.txt' file will be the 2D matrix with fixed width columns, since the dimensions are adjusted in terms of a pixel size, no axis values are needed to get the correct proportions from the image.
   * The BMP file can only come from the formatted matrix, and provides the image processed as a bitmap, the color is set to show orange shades depending on the intensity values, although black and some other colors may appear under certain conditions.

***NOTE 1**: The aspect ratio might be slightly different if one of the dimensions is a multiple of 4 and the other one is not.

***NOTE 2**: In this release the coordinate map should have one point at the coordinate (0,0) to avoid unusual behavior. Future work aims to fix this inconvenience in a future version.

As mentioned above, this program uses the header file "spectrum_map.hpp" where input, output and processing functions are written. The header file can be used independently for custom software if desired.

## **Using spectrumview**

The general syntax to run the program is:

`./spectrumview + 'Path to directory' +  format + intensity_mode + channels (only for integrated mode) + output_file + energy`

### **Getting an spectrum map with intensity extracted by the linear interpolation method**

The syntax for the command line is the following:

`./spectrumview + 'Path to directory' +  format + interpolated + output_file + energy`

1. 'Path to directory': Must be a path to a directory not to a file, where all the acquired spectra for the map is stored.

2. Format has 4 options:

   * All: Creates the files of both the raw and formatted grid and the BMP file.

   * Raw: Creates only the files associated to the raw map to plot in a third party software.

   * Grid: Creates only the file from the formatted grid to plot in a third party software.

   * bmp: Creates only the BMP file.

3. Intensity_mode is: interpolated
   * Looks for the nearest energies below and above the requested energy value, extrapolation is not supported. To calculate the intensity at the requested energy, linear interpolation is used:
  $$intensity = intensity_{0}+ \frac{(energy - energy_{0}) * (intensity_{1} - intensity_{0})}{(energy_{1}-energy_{0})} $$

4. Output_file: The output name can be generic and include the energy of interest, depending on the file the program will modify the name to identify each file.

5. The requested energy is the last argument.

Example:

```./spectrumview 'C:/Users/ID/Documents/Experiments/EELS Map files' all interpolated EELS_map_example 0.035```

In this case, 5 output files will be created: the raw map as a matrix in a text file, the x and y axis values as two separate text files, the formatted map as a matrix in a different text file and the BMP file with the RGB figure. Both maps intensities are defined from an interpolation to get the intensity at 0.096 eV.

**Syntax for extraction with integrated_intensity:**

`./spectrumview + 'Path to directory' +  format + integrated + channels + output_file + energy`

1. 'Path to directory': Must be a path to a directory not to a file.

2. Format has 4 options:

   * All: Creates the files of both the raw and formatted grid and the BMP file.

   * Raw: Creates only the files associated to the raw map to plot in a third party software.

   * Grid: Creates only the file from the formatted grid to plot in a third party software.

   * bmp: Creates only the BMP file.

3. Intensity_mode is: integrated.
   * The integration is performed as the sum of all the intensity values within the integration limits.

4. The channels to consider for the range must be specified.
     * If the energy value is in the middle of the container: the integration window goes from $energy - channels$ to $energy + channels$.
     * If the energy is at the beginning or the end of the energy axis the integration window is $energy \pm channels$ considering only the range where energy values exist.
     * If the energy is not at the edge but the integration window has energy values that don't exist in the energy axis, the integration window will be asymmetric and will consider only the existing energy values.

5. Output_file: The output name can be generic and include the energy of interest, depending on the file the program will modify the name to identify each file.

6. The requested energy is the last argument.
   * The requested energy is approximated, the program will use the first element that is larger or equal to the requested energy to perform the integration.

Example:

```./spectrumview.cpp 'C:/Users/ID/Documents/Experiments/EELS Map files' bmp integrated 3 map_one 0.096```

For this example, the integration window will consider 3 energy values above and 3 energy values below 0.096 eV assuming that the energy axis goes from 0 to 1 in steps of 0.005 eV. The output file will be the bitmap.

## The header file spectrum_map.hpp

There are 3 main elements within this header file: the input functions, the experimental objects and  the output functions.

### **Input functions**

### *Only the first function is explicitly used in the program, the rest are used in constructors.*

* `opendirectory (const std::string &path)`: This function takes a path to a*directory* where all the data files should be stored. It reads through the folder using the std::filesystem library and stores the filenames in a vector.

  1. Arguments - A string with the path to the directory where the files are stored.
  2. Returns - A `std::vector` with the path to all the files within the directory.  

* `readfile (const std::filesystem::path &path, const std::string &axis)`: This function takes a path to a *file* and reads through its content to fill up two vector containers: one will store the energy (or frequency) values and the other will store the measured intensities. To properly read the file, it needs to follow a structure where there is only a pair of values per line separated by a tab or space and with no additional spaces at the end or the beginning of the file. The function can read signed floats and it actually stores each value as a double. This function returns either the energy axis vector or the intensity vector.

  1. Arguments - Path to a file, the program takes the paths from the output vector of the `readfile` function; Specify axis to get as an output, can be energy or intensity.
  2. Returns - `std::vector<double>` with the values extracted from the file for the energy or the intensity axis.

* `findcoords (const std::filesystem::path &path, const std::string &coordinate)`: This function reads the name of the file to extract the position where the data was acquired with respect to pre-designed cartesian map. The cartesian map for a given experiment must be developed in the experimental design process and therefore the positions where the probe was located during the experiments are known and should be explicitly indicated in the file name as the two las elements of the filename before the extension separated by dashes. The coordinates are stored as a double and can be returned one at a time. For each file two calls to this function should exist.

  1. Arguments - Path to a file, the program takes the paths from the output vector of the `readfile` function; direction to return: x or y.
     * A proper naming for the file looks like this:

        ```'EELS-data-SiC-flake-10nm-130nm.dat'.```

        Our group uses the .dat file extension to save spectra but you can use any extension as long as the name is correct and the format of the file can be read properly by the program.

  2. Returns - `double` with the position from the direction x or y as requested by the program.

### **Classes**

#### **`Class spectrum`**

* constructor(`const std::filesystem::path &path`): The spectrum constructor makes use of the readfile and the findcoords function to create an object that consists of two vectors: one for the energy and one for the intensity and two points x and y. Since it uses the previously shown functions, the constructor takes a path that is then used as an input.

  1. Arguments - Path to the file with the spectrum data.

#### *Member Functions of `spectrum` class*

* `integrated_intensity (const double &energy, const uint64_t &channels)`: To extract the intensities for the contour map the `integrated_intensity` function looks up for the first value that is equal or greater than the requested energy and then adds the contiguous values of intensity above and below taking a range from energy - channels to energy + channels. If the energy requested is at the beginning or at the end, the integration occurs only in the direction where values are available and a similar case occurs if the range goes out of bounds from the vector.

  1. Arguments - The energy of interest using the same units that are used in the spectrum file; the number of channels to define the integration window.
  2. Returns - `double` the result of summing the intensities in the corresponding range following the guideline explained above.

* `interpolated_intensity (const double &energy)`: To extract the intensities for the contour map the interpolated_intensity function looks for the two values where the energy of interest would lie between and then performs a simple linear interpolation to find the intensity at the input energy.

  1. Arguments - The energy of interest using the same units that are used in the spectrum file.
  2. Returns - `double` The intensity at the energy of interest calculated from interpolation.

* `show_position (const std::string &pos)`: This function returns either the abscissa or the ordinate as specified by pos.

  1. Arguments - pos specifies the direction `"x"` or `"y"`.
  2. Returns - `double` the value of x or y as specified by pos.

#### **`Class data_map`**

* constructor `(const std:set<std:tuple <double,double>> &keys, const std::map<<std:tuple <double,double>,double> &filler)`: To use the data_map object it is needed to create a set of tuples and a map with a tuple as keys and the intensity as value. These two are made in the main function and should not have repeated values (hence the use of an ordered set and map).

    A `data_map` object gets constructed by storing the width and length of the original data, based on the 2D shape formed by the combination of all the x and y values of all the files. It also stores two vectors with all the unique values of x and y. Two additional vectors store the step size for the pixels in x and y considering the difference between contiguous unique values.

    The last element of the `data_map` class is a flattened matrix that has the shape of the 2D map, this matrix sets the spatial distribution of the intensity and pads zeros in the coordinates that get constructed but have no data assigned.

  1. Arguments - A set of `<std::tuple>` that stores the keys of (x,y) coordinates to access values in the map; A map with `std::tuple` with the coordinates as key and a `double` that corresponds to the extracted i intensity values associated to an (x,y) coordinate.

#### *Member functions of `data_map` class*

* `show_axis (const std::string &axis)`: The `show_axis` function returns the container with the unique values for either the x or y axis as requested in the program.

  1. Arguments - Specify the direction of the axis: either `"x"` or `"y"`.
  2. Returns - `std::vector<double>` with the unique values along x or y.

* `show_raw ()`: The raw map constructed is likely not suitable for a BMP file, however the `show_raw` function returns the flattened matrix with only the raw data. An output function can read it and write it into a '.txt' file to manipulate and visualize with additional resources (third party libraries, software, other code languages).

    1. Returns - `std::vector<double>` 2D flattened matrix with only the raw data.

* `show_dimensions(const std::string &size_direction)`: The `show_dimensions` function returns the true width or the true length stored in the `data_map` private members to be read. The orientation is specified as an argument.

  1. Arguments - Specify the direction of the dimension of interest can be `"width"` or `"length"`.
  2. Returns - `uint32_t` with the size of the specified dimension.

* `show_formatted_grid()`: A function that takes the raw map, and resizes it to have even pixel sized steps with the required characteristics to build a BMP file. It returns a 2D flattened matrix with the resized shape. Intensity is filled making sure to keep the information from the raw map.

  1. Returns - `std::vector<double>` 2D flattened matrix with the resized dimensions and suitable for BMP file.  

* `show_formatted_dimensions (const std::string &size_direction)`: Returns the result of the computation for the resizing of the 2D matrix to create the formatted grid. Its function is the same to show_dimensions for the resized matrix.

  1. Arguments - Specify the direction of the dimension of interest can be `"width"` or `"length"`.
  2. Returns - uint32_t with the size of the specified dimension.

#### **`Class BmpHeader`**

The BmpHeader stores metadata required for the binary BMP file.

* constructor `(const uint64_t &width, const uint64_t &length)`: The constructor updates the metadata to estimate the bit size with the width and height from the arguments.

  1. Arguments - Specify the dimensions of width and length of the image. Must be multiples of 4.

#### *Member functions of `BmpHeader`*

* `write_header (std::ofstream &output_file)` The write header writes the first section of the binary BMP file into a created file.  

  1. Arguments - The file where the bitmap will be written.
  2. Returns - No return in this function. Writes the BmpHeader in a file.

#### **'Class BmpInfoHeader'**

The BmpInfoHeader creates the second section of metadata required for the binary BMP file.

* constructor `(const uint64_t &width, const uint64_t &length)` The constructor resize the pixel width and height of the figure with the calculated values of the formatted grid or from the figure to build.
  1. Arguments - Specify the dimensions of width and length of the image. Must be multiples of 4.

#### *Member functions of `BmpInfoHeader`*

* `write_infoheader (std::ofstream &output_file)` The write header writes the second section of the binary BMP file into a created file.

  1. Arguments - The file where the bitmap will be written.
  2. Returns - No return in this function. Writes the BmpInfoHeader in a file.

### **Output functions**

* `external_plot(const std::vector <double> &map, const uint64_t &width, const uint64_t &length,std::string &output_title)`: This function reads the 2D flattened matrix of either the raw map and the formatted grid and writes them as a 2D matrix in a .txt file with fixed width columns. Map, width and length must be properly sized. The last argument indicates the name of the file without extension.

  1. Arguments - The 2D flattened matrix contained on a vector; the width is the column size of the matrix; the height is the row size of the matrix. Specify identification of the file, the output file name.
  2. Creates a .txt file with a 2D matrix with fixed column width.

* `external_plot_axis (std::vector <double> &x, std::vector <double> &y, std::string &output_title)`: Writes the values of x and y in two independent .txt files, one for each axis. Only applies to raw_map, this files can be useful to plot the raw_map assuming there is uneven spacing between the points.

  1. Arguments - The values for the x and y axis of the map contained in a `std::vector`.
  2. Creates two .txt files for x and y. Modifies the output title to specify the information contained in the file.

* `build_bitmap (std::vector<double> &intensity, const uint64_t &width,const uint64_t &height, std::string output_title)`: This function takes the flattened grid, width and height and creates a coloured binary BMP file. This section was written with the aid of multiple sources online but mainly following guidelines from: <https://dev.to/muiz6/c-how-to-write-a-bitmap-image-from-scratch-1k6m>.

  1. Arguments - A vector with the 2D flattened matrix of the intensity map with proper dimensions to create a bitmap; the width is the column size of the matrix; the height is the row size of the matrix. Specify the title of the output file.
  2. Creates a BMP file with the intensity map as a bitmap.

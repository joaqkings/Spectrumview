# Spectrum View

By Joaquin Reyes

## **About spectrumview.cpp**

### Spectrum View is a C++ program to build spatially resolved spectroscopy measurements, especially focused on Electron Energy Loss Spectroscopy experiments. A variety of techniques can take advantage of what this program offers, as the EELS results can be similar to SNOM, Raman Microscopy, IR Microscopy, etc. This program can create two kinds of files: '.txt' files that can later be processed in other languages or software and BMP files showing the spatial distribution of the intensity at a given energy

### It uses a header file "spectrum_map.hpp" where input, output and processing functions are written. The header file can be used independently for custom software if desired

## The header file spectrum_map.hpp

### There are 3 main elements within this header file: the input functions, the experimental objects and  the output functions

### **Input functions**

### *Only the first function is explicitly used in the program, the rest are used in constructors*

*opendirectory (path): This function takes a path to a*directory* where all the data files should be stored. It reads through the folder using the std::filesystem library and stores the filenames in a vector

1. Arguments - (path to a directory)
2. Returns - std::vector<fs::path> with the path to all the files within the directory
   
*readfile (path): This function takes a path to a*file* and reads through its content to fill up two vector containers: one will store the energy (or frequency) values and the other will store the measured intensities. To properly read the file, it needs to follow a structure where there is only a pair of values per line separated by a tab or space and with no additional spaces at the end or the beginning of the file. The function can read signed floats and it actually stores each value as a double. This function returns either the energy axis vector or the intensity vector.

1. Arguments - (path to a file, axis to return: energy or intensity)
2. Returns - std::vector<double> with the values extracted from the file for the energy or the intensity axis
   
*findcoords (path, coordinate): This function reads the name of the file to extract the position where the data was acquired with respect to pre-designed cartesian map. The cartesian map for a given experiment must be developed in the experimental design process and therefor the positions were the probe was located during the experiments are known and should be explicitly indicate in the file name as the two las elements of the filename before the extension separated by dashes. The coordinates are stored as a double and can be returned one at a time. For each file two calls to this function should exist

1. Arguments - (path, axis to return: x or y)
2. A proper naming for the file looks like this: 'EELS-data-SiC-flake-10nm-130nm.dat'. Our laboratory uses the .dat file extension to save spectra but it's not necessary as long as the format of the name is correct
3. Returns - double with the position x or y as requested by the program
   
### **Classes**

#### **Class spectrum**

*constructor(path) The spectrum constructor makes use of the readfile and the findcoords function to create an object that consists of two vectors: one for the energy and one for the intensity and two points x and y. Since it uses the previously shown functions, the constructor takes a path that is then used as an input

1. Arguments - (path)

#### *Member Functions*

*integrated_intensity (energy, channels) To extract the intensities for the contour map the integrated_intensity function looks up for the first value that is equal or greater than the requested energy and then adds the contiguous values of intensity above and below taking a range from energy - channels to energy + channels. If the energy requested is at the beginning or at the end, the integration occurs only in the direction where values are available and a similar case occurs if the range goes out of bounds from the vector

1. Arguments - (energy -of interest-, channels -for the range-)
2. Returns - double the result of summing the intensities in the corresponding range following the guideline explained above

*interpolated_intensity (energy) To extract the intensities for the contour map the interpolated_intensity function looks for the two values where the energy of interest would lie between and then performs a simple linear interpolation to find the intensity at the input energy

1. Arguments - (energy -of interest-)
2. Returns - double The intensity at the energy of interest calculated from interpolation.

*show_position (pos) This function returns either the abscissa or the ordinate as specified by pos

1. Arguments - (pos: can be either x or y)
2. Returns - double the value of x or y as specified by pos

#### **Class data_map**

*constructor (keys, filler) To use the data_map object it is needed to create a set of tuples and a map with a tuple as keys and the intensity as value. These two are made in the main function and should not have repeated values (hence the use of an ordered set and map)

A data_map object gets constructed by storing the width and length of the original data, based on the 2D shape formed by the combination of all the x and y values of all the files. It also stores two vectors with all the unique values of x and y. Two additional vectors store the step size for the pixels in x and y considering the difference between contiguous unique values

The last element of the data_map class is a flattened matrix that has the shape of the 2D map, this matrix sets the spatial distribution of the intensity and pads zeros in the coordinates that get constructed but have no data assigned.

1. Arguments - (set <std::tuple> keys tuple with (x,y) coordinates, <std::tuple, double> filler with intensity values associated to an (x,y) coordinate)

#### *Member functions*

*show_axis (axis) The show_axis function returns the container with the unique values for either the x or y axis as requested in the program

1. Arguments - (axis either x or y)
2. Returns - std::vector with the unique values along x or y

*show_raw () The raw map constructed is likely not suitable for a BMP file, however the show_raw function returns the flattened matrix with only the raw data. An output function can read it and write it into a .txt file to manipulate and visualize with additional resources (third party libraries, software, other code languages)

1. Returns - std::vector 2D flattened matrix with only the raw data

*show_dimensions(size_direction) The show_dimensions function returns the true width or the true length stored in the data_map private members to be read. The orientation is specified as an argument

1. Arguments - (size_direction: can be width or length)
2. Returns - uint32_t with the size of the specified dimension

*show_formatted_grid() A function that takes the raw map, and resizes it to have even pixel sized steps with the required characteristics to build a BMP file. It returns a 2D flattened matrix with the resized shape. Intensity is filled accordingly to keep the information from the raw map

1. Returns - std::vector 2D flattened matrix with the resized dimensions and suitable for BMP file
   
*show_formatted_dimensions (size_direction) Returns the result of the computation for the resizing of the 2D matrix to create the formatted grid. Its function is the same to show_dimensions for the resized matrix

1. Arguments - (size_direction: can be width or length)
2. Returns - uint32_t with the size of the specified dimension

#### **Class BmpHeader**

The BmpHeader stores metadata required for the binary BMP file

*constructor (width, length) The constructor updates the metadata to estimate the bit size with the width and height from the arguments

#### *Member functions*

*write_header (output_file) The write header writes the first section of the binary BMP file into a created file. 

1. Arguments - (std::ofstream output_file) The file where the bitmap will be written
2. Returns - No return in this function. Writes the BmpHeader in a file

#### **Class BmpInfoHeader**

The BmpInfoHeader creates the second section of metadata required for the binary BMP file

*constructor (width, height) The constructor resize the pixel width and height of the figure with the calculated values of the formatted grid

#### *Member functions*

*write_infoheader (output_file) The write header writes the second section of the binary BMP file into a created file.

1. Arguments - (std::ofstream output_file) The file where the bitmap will be written
2. Returns - No return in this function. Writes the BmpInfoHeader in a file

### **Output functions**

*external_plot(map, width, length, output_title) This function reads the 2D flattened matrix of either the raw map and the formatted grid and writes them as a 2D matrix in a .txt file with fixed width columns. map, width and length must be properly sized. The last argument indicates the name of the file without extension

1. Arguments - (map: 2D flattened matrix, width, length, output_title)
2. Creates a .txt file with a 2D matrix with fixed column width

*external_plot_axis (x, y) Writes the values of x and y in two independent .txt files, one for each axis. Only applies to raw_map, this files can be useful to plot the raw_map assuming there is uneven spacing between the points

1. Arguments - (std::vector x, std::vector y)
2. Creates two .txt files for x and y.
   
*build_bitmap (intensity, width, height, output_title) This function takes the flattened grid, width and height and creates a coloured binary BMP file. This section was written with the aid of multiple sources online but mainly following guidelines from: <https://dev.to/muiz6/c-how-to-write-a-bitmap-image-from-scratch-1k6m>

1. Arguments (intensity: 2D flattened formatted grid, width, height, output_title)
2. Creates a BMP file with the intensity map as a bitmap.


## **Using spectrumview.cpp**

### The cpp program uses the aforementioned functions and classes to produce the desired files by command line arguments. The syntax depends on the type of intensity extraction required 

**Syntax for extraction with interpolated_intensity:**

.\spectrumview.cpp + 'Path to directory' +  format + intensity_mode + output_file + energy

'Path to directory': Must be a path to a directory not to a file.

Format has 4 options:

 -All: Creates the files of both the raw and formatted grid and the BMP file

 -Raw: Creates only the files associated to the raw map to plot in a third party software

 -Grid: Creates only the file from the formatted grid to plot in a third party software

 -bmp: Creates only the BMP file

 Intensity_mode is: interpolated

 Output_file: The output name can be generic and include the energy of interest, depending on the file the program will modify the name to identify each file

 The energy of interest is the last argument

Example:
.\spectrumview.cpp 'C://Users/ID/Documents/Experiments/EELS Map files' all interpolated map_one 0.096\

**Syntax for extraction with integrated_intensity:**

.\spectrumview.cpp + 'Path to directory' +  format + intensity_mode + channels + output_file + energy

'Path to directory': Must be a path to a directory not to a file.

Format has 4 options:

 -All: Creates the files of both the raw and formatted grid and the BMP file

 -Raw: Creates only the files associated to the raw map to plot in a third party software

 -Grid: Creates only the file from the formatted grid to plot in a third party software

 -bmp: Creates only the BMP file

 Intensity_mode is: integrated

 The channels to consider for the range must be specified. If the energy value is in the middle of the container range goes from energy - channels to energy + channels. Otherwise it truncates the range to the size of the vector.

 Output_file: The output name can be generic and include the energy of interest, depending on the file the program will modify the name to identify each file

 The energy of interest is the last argument

Example:
.\spectrumview.cpp 'C://Users/ID/Documents/Experiments/EELS Map files' all integrated 3 map_one 0.096
   
 The Map.zip is a test folder that can be used with spectrum_view.cpp

#pragma once

/**
 * @file spectrum_map.hpp
 * @author Joaquin Reyes (reyesgoj@mcmaster.ca)
 * @brief spectrum_map: A library for image processing from data acquisition in spatially resolved spectroscopy. This header file contains the main classes spectra and data_map, the input/output functions and the BMP file generator.
 * @version 0.1
 * @date 2022-12-23
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace fs = std::filesystem;

// ==================================================================================================== //
//                                        INPUT FUNCTIONS                                               //

// Function to read a string with a path to a directory to open it and get the file names.
/**
 * @brief Takes a path of a directory and creates a container with the file names.
 *
 * @param path Path to the directory where the data files are saved.
 * @return Returns a vector that contains the individual file names.
 */
std::vector<fs::path> opendirectory(const std::string &path)
{
    fs::path p = path;
    std::vector<fs::path> directory;
    try
    {
        for (fs::directory_entry const &dir_entry : fs::directory_iterator{p})
            directory.push_back(dir_entry.path());
    }
    catch (fs::filesystem_error const &ex)
    {
        std::cout << "Input path error: " << ex.code().message();
        exit(0);
    }
    return directory;
}

/**
 * @brief Reads an individual data file to create the containers for the energy/frequency or intensity axis.
 *
 * @param path The path to the file where the information will be extracted. If you are using spectrumview, the program creates this path.
 * @param axis The axis to be returned. Can be either "energy" or "intensity".
 * @return Returns a vector containing the energy or intensity values. If an error occurs, terminates the program.
 */
std::vector<double> readfile(const fs::path &path, const std::string &axis)
{
    std::ifstream path_input(path);
    if (!path_input.is_open())
    {
        std::cout << "Can't open a file!:" << path.string();
        exit(0);
    }

    std::string line;
    std::vector<double> axis_container;
    while (getline(path_input, line))
    {
        for (const char &c : line)
        {
            if (isalpha(c))
            {
                throw std::invalid_argument("Error reading the file " + path.string() + ": Eliminate alphabetic characters from the energy values.");
            }
            else if (ispunct(c))
            {
                if (c == '-' or c == '.')
                    continue;
                else
                    throw std::invalid_argument("Error reading the file " + path.string() + ": Eliminate punctuation characters. Only negation '-' at the beginning or a single point '.' for a float are allowed.");
            }
            else if (isspace(c) and std::count(line.begin(), line.end(), ' ') != 1)
            {
                throw std::invalid_argument("Error reading the file " + path.string() + ": Eliminate spaces within the values or eliminate additional values, each line should have only a pair of values separated by spaces.");
            }
        }
        uint64_t tab = line.find(' ');
        if (std::count(line.begin(), line.end(), ' ') > 1)
        {
            throw std::invalid_argument("Error reading the file " + path.string() + ". There might be more than two elements per line or spaces at the end of a line");
            exit(0);
        }
        else if (tab == line.size() - 1 or tab == std::string::npos)
        {
            std::cout << "File " + path.string() + " is missing one or more values in a column.";
            exit(0);
        }
        if (!axis.find("energy"))
        {
            std::string energy = line.substr(0, tab);
            if ((energy.find('-') != 0 and energy.find('-') != std::string::npos) or std::count(energy.begin(), energy.end(), '-') > 1 or std::count(energy.begin(), energy.end(), '.') > 1)
                throw std::invalid_argument("Error reading the file " + path.string() + ": Eliminate punctuation characters. Only negation '-' at the beginning or a single point '.' for a float are allowed.");
            else
                axis_container.push_back(stod(energy));
        }
        else if (!axis.find("intensity"))
        {
            std::string intensity = line.substr(tab + 1, line.size());
            if ((intensity.find('-') != 0 and intensity.find('-') != std::string::npos) or std::count(intensity.begin(), intensity.end(), '-') > 1 or std::count(intensity.begin(), intensity.end(), '.') > 1)
                throw std::invalid_argument("Error reading the file " + path.string() + ": Eliminate punctuation characters. Only negation '-' at the beginning or a single point '.' for a float are allowed.");
            else
                axis_container.push_back(stod(intensity));
        }
    }
    path_input.close();
    if (axis_container.empty())
    {
        std::cout << "An error occurred while reading the file " + path.string() + ": File may be empty! ";
        exit(0);
    }
    return axis_container;
}

/**
 * @brief Identifies the coordinates where the spectrum was acquired from the filename. Format of filename must be 'file_id-x_coordinate-y_coordinate.extension'.
 *
 * @param path The path to the file where the information will be extracted. If you are using spectrumview, the program creates this path.
 * @param coordinate Specify wether the ordinate or abscissa. Can be either "x" or "y".
 * @return Returns a double with the coordinate value required.
 */
double findcoords(const fs::path &path, const std::string &coordinate)
{
    fs::path filename = path.stem();
    std::string file = filename.string();
    uint64_t yseparator = file.rfind("-");
    std::string _y = file.substr(yseparator + 1, file.size());
    file = file.substr(0, yseparator);
    uint64_t xseparator = file.rfind("-");
    std::string _x = file.substr(xseparator + 1, file.size());
    if (coordinate == "x")
    {
        for (std::string::iterator i = _x.begin(); i < _x.end(); i++)
        {
            if (ispunct(*i))
                throw std::invalid_argument("Unrecognized character for x position in file: " + path.string());
            else if (isalpha(*i))
            {
                uint64_t start = _x.find(*i);
                if (*i == 'p')
                {
                    _x.replace(start, 1, ".");
                }
                else
                {
                    _x.erase(start, 1);
                    i--;
                }
            }
        }
        if (_x.empty())
            throw std::invalid_argument("No value specified for position x inf file: " + path.string());
        else
            return stod(_x);
    }
    else if (coordinate == "y")
    {
        for (std::string::iterator i = _y.begin(); i < _y.end(); i++)
        {
            if (ispunct(*i))
                throw std::invalid_argument("Unrecognized character for y position in file: " + path.string());
            else if (isalpha(*i))
            {
                uint64_t start = _y.find(*i);
                if (*i == 'p')
                {
                    _y.replace(start, 1, ".");
                }
                else
                {
                    _y.erase(start, 1);
                    i--;
                }
            }
        }
        if (_y.empty())
            throw std::invalid_argument("No value specified for position y in file: " + path.string());
        else
            return stod(_y);
    }
    else
        throw std::invalid_argument("Specify position of interest. Can only be 'x' or 'y'");
}

//                                           End input functions
// ====================================================================================================== //
//                                           Begin class spectrum                                         //
/**
 * @brief Class to store the information from the data files. Contains the energy, intensity and spatial location.
 */
class spectrum
{

public:
    /**
     * @brief Construct a new spectrum object from a file
     *
     * @param path Takes the path to a file were the information will be extracted. If you are using spectrumview, the program creates this path.
     */
    spectrum(const fs::path &path)
    {
        energy_ax = readfile(path, "energy");
        intensity = readfile(path, "intensity");
        pos_x = findcoords(path, "x");
        pos_y = findcoords(path, "y");
    }

    /**
     * @brief Extracts the intensity at a given energy by locating the nearest upper value and adding the intensities from contiguous specified amount of pixels. If energy is first or last value only channels within the axis are considered.
     *
     * @param energy Energy to be mapped.
     * @param channels The amount of channels to integrate per side.
     * @return Returns a long float with the result of the sum of the intensities for each channel that was considered.
     */
    double integrated_intensity(const double &energy, const uint64_t &channels)
    {
        double integrated_intensity = 0;
        size_t pos = 0;
        size_t lower_limit;
        size_t upper_limit;
        if (energy_ax[0] > energy or energy > energy_ax[energy_ax.size() - 1])
        {
            std::cout << "Error: Requested energy value was not found. A file may not contain the energy value you requested.";
            exit(0);
        }
        for (uint64_t i = 0; i < energy_ax.size(); i++)
        {
            if (energy_ax[i] >= energy)
            {
                pos = i;
                break;
            }
        }
        if (pos == 0 or channels > pos)
        {
            lower_limit = 0;
            upper_limit = pos + channels + 1;
        }
        else if (pos == energy_ax.size() - 1 or ((pos + channels) >= energy_ax.size() - 1))
        {
            lower_limit = pos - channels;
            upper_limit = energy_ax.size();
        }
        else
        {
            lower_limit = pos - channels;
            upper_limit = pos + channels;
        }
        for (uint64_t i = lower_limit; i < upper_limit + 1; i++)
        {
            integrated_intensity += intensity.at(i);
        }
        return integrated_intensity;
    }

    /**
     * @brief Extracts the intensity at a exact required energy by using linear interpolation to find the intensity from known values.
     *
     * @param energy Energy to be mapped.
     * @return Returns a double with the intensity for the requested energy.
     */
    double interpolated_intensity(const double &energy)
    {
        double int_intensity = 0;
        size_t pos = 0;
        size_t lower_limit;
        size_t upper_limit;
        if (energy_ax[0] > energy or energy > energy_ax[energy_ax.size() - 1])
        {
            std::cout << "Error: Requested energy value was not found. A file may not contain the energy value you requested.";
            exit(0);
        }
        for (uint64_t i = 0; i < energy_ax.size(); i++)
        {
            if (energy_ax[i] > energy)
            {
                pos = i;
                break;
            }
        }
        lower_limit = pos - 1;
        upper_limit = pos + 1;

        int_intensity = intensity.at(lower_limit) + (((energy_ax.at(pos) - energy_ax.at(lower_limit)) * (intensity.at(upper_limit) - intensity.at(lower_limit))) / (energy_ax.at(upper_limit) - energy_ax.at(lower_limit)));
        return int_intensity;
    }

    /**
     * @brief Prints either the x or the y coordinate as requested. This is handled by spectrumview.cpp.
     *
     * @param pos The coordinate of interest. Either "x" or "y".
     * @return Returns a double with the ordinate or the abscissa.
     */
    double show_position(const std::string &pos)
    {
        double position;
        if (pos == "x")
        {
            position = pos_x;
            return position;
        }
        else if (pos == "y")
        {
            position = pos_y;
            return position;
        }
        else
            throw std::invalid_argument("Position can only be for x and y coordinates.");
    }

private:
    /**
     * @brief Vector to store the energy/frequency values.
     */
    std::vector<double> energy_ax;
    /**
     * @brief Vector to store the intensity values.
     */
    std::vector<double> intensity;
    /**
     * @brief Variables to store the x-coordinate.
     */
    double pos_x = 0;
    /**
     * @brief Variables to store the y-coordinate.
     */
    double pos_y = 0;
};

//                                            End class spectrum                                          //
//========================================================================================================//
//                                            Begin class data_map                                        //

/**
 * @brief Class to create the energy map. Can provide a raw map with a "pixel" per point acquired, or a map with added pixels to improve aspect.
 */
class data_map
{
public:
    /**
     * @brief Construct a new data map object. In spectrumview, a map with coordinates as keys is created to fill the raw map.
     *
     * @param keys The coordinates extracted from the file name for all the data files.
     * @param intensity_fill The intensity values for a spectrum map associated with their respective coordinates.
     */
    data_map(const std::set<std::tuple<double, double>> &keys, const std::map<std::tuple<double, double>, double> &intensity_fill)
    {
        if (keys.empty() or intensity_fill.empty())
        {
            std::cout << "Error while processing the files";
            exit(0);
        }

        for (std::set<std::tuple<double, double>>::iterator i = keys.begin(); i != keys.end(); i++)
        {
            x_handle.push_back(std::get<0>(*i));
            y_handle.push_back(std::get<1>(*i));
        }

        std::sort(x_handle.begin(), x_handle.end());
        std::sort(y_handle.begin(), y_handle.end());
        auto last_x = std::unique(x_handle.begin(), x_handle.end());
        auto last_y = std::unique(y_handle.begin(), y_handle.end());
        x_handle.erase(last_x, x_handle.end());
        y_handle.erase(last_y, y_handle.end());
        true_width = (uint32_t)x_handle.size();
        true_length = (uint32_t)y_handle.size();

        for (uint64_t i = 1; i < x_handle.size(); i++)
        {
            uint32_t step_size = static_cast<uint32_t>(std::llround(x_handle.at(i) - x_handle.at(i - 1)));
            x_step.push_back(step_size);
        }

        for (uint64_t i = 1; i < y_handle.size(); i++)
        {
            uint32_t step_size = static_cast<uint32_t>(std::llround(y_handle.at(i) - y_handle.at(i - 1)));
            y_step.push_back(step_size);
        }

        for (std::vector<double>::iterator i = y_handle.begin(); i < y_handle.end(); i++)
        {
            for (std::vector<double>::iterator j = x_handle.begin(); j < x_handle.end(); j++)
            {
                std::tuple<double, double> coordinates = std::make_tuple(*j, *i);
                if (intensity_fill.contains(coordinates))
                    raw_map.push_back(intensity_fill.at(coordinates));
                else
                    raw_map.push_back(0);
            }
        }
    }

    /**
     * @brief Returns a vector with all the values for the abscissa and/or ordinate of the map.
     *
     * @param axis Specify wether you want the file with the abscissa or the file of the ordinate.
     * @return Returns a vector with all the values of the true coordinates in a specific direction.
     */
    std::vector<double> show_axis(const std::string &axis)
    {
        if (axis == "x")
            return x_handle;
        else if (axis == "y")
            return y_handle;
        else
            throw std::invalid_argument("Only axis 'x' or 'y' can be requested");
    }

    /**
     * @brief Extracts the flattened matrix with the intensities extracted from the files respecting the dimensions of the experiment.
     *
     * @return Returns a flattened matrix with the intensity values extracted from the map, doesn't consider if the step size varies.
     */
    std::vector<double> show_raw()
    {
        std::vector<double> raw_image(raw_map.size());
        for (uint64_t i = 0; i < raw_map.size(); i++)
            raw_image[i] = raw_map[i];
        return raw_image;
    }

    /**
     * @brief Provides access to read the dimensions of raw image.
     *
     * @param size_direction Specify wether the width or length is needed.
     * @return Returns a 32-bit integer with the width or length measured in data points.
     */
    std::uint32_t show_dimensions(const std::string &size_direction)
    {
        if (size_direction == "width")
            return true_width;
        else if (size_direction == "length")
            return true_length;
        else
            throw std::invalid_argument("Can't access requested dimension.");
    }

    /**
     * @brief Creates a matrix with the intensities extracted from the files, and adds pixels to create a uniform pixel size and to allow the build of a BMP figure.
     *
     * @return Returns a vector with the flattened matrix with the intensity values extracted from the raw map. Creates a uniform spacing.
     */
    std::vector<double> show_formatted_grid()
    {
        if (x_handle.at(0) != 0 or y_handle.at(0) != 0)
        {
            std::cout << "Please note that if the initial point of coordinates is not (0,0) unusual behaviour may occur with the bitmap and formatted grid on this release." << '\n'
                      << "Future work will look for a way to fix such inconvenience." << '\n';
        }

        uint32_t width = (((uint32_t)x_handle.at((size_t)std::distance(x_handle.begin(), std::max_element(x_handle.begin(), x_handle.end()))) - (uint32_t)x_handle.at((size_t)std::distance(x_handle.begin(), std::min_element(x_handle.begin(), x_handle.end())))) / x_step.at((size_t)std::distance(x_step.begin(), std::min_element(x_step.begin(), x_step.end())))) + 1;
        uint32_t length = (((uint32_t)y_handle.at((size_t)std::distance(y_handle.begin(), std::max_element(y_handle.begin(), y_handle.end()))) - (uint32_t)y_handle.at((size_t)std::distance(y_handle.begin(), std::min_element(y_handle.begin(), y_handle.end())))) / y_step.at((size_t)std::distance(y_step.begin(), std::min_element(y_step.begin(), y_step.end())))) + 1;

        if (width % 4 != 0)
        {
            width = (width + 4 - (width % 4));
        }
        if (length % 4 != 0)
        {
            length = (length + 4 - (length % 4));
        }

        std::vector<uint32_t> x_pixel_step;
        uint32_t accumulated = 0;
        for (std::vector<uint32_t>::iterator i = x_step.begin(); i < x_step.end(); i++)
        {

            uint32_t pixel_step = (*i * width) / ((uint32_t)x_handle.at((size_t)std::distance(x_handle.begin(), std::max_element(x_handle.begin(), x_handle.end()))) - (uint32_t)x_handle.at((size_t)std::distance(x_handle.begin(), std::min_element(x_handle.begin(), x_handle.end()))));
            accumulated += pixel_step;
            x_pixel_step.push_back(accumulated);
        }

        std::vector<uint32_t> y_pixel_step;
        accumulated = 0;
        for (std::vector<uint32_t>::iterator i = y_step.begin(); i < y_step.end(); i++)
        {
            uint32_t pixel_step = (*i * length) / ((uint32_t)y_handle.at((size_t)std::distance(y_handle.begin(), std::max_element(y_handle.begin(), y_handle.end()))) - (uint32_t)y_handle.at((size_t)std::distance(y_handle.begin(), std::min_element(y_handle.begin(), y_handle.end()))));
            accumulated += pixel_step;
            y_pixel_step.push_back(accumulated);
        }

        std::vector<double> formatted_grid(width * length);
        uint64_t m = 0;
        uint64_t n = 0;
        uint64_t raw_x = 0;
        uint64_t raw_y = 0;
        for (uint64_t i = 0; i < length; i++)
        {
            if (i == y_pixel_step[n])
            {
                raw_y++;
                n++;
            }
            for (uint64_t j = 0; j < width; j++)
            {
                if (j == x_pixel_step[m])
                {
                    raw_x++;
                    m++;
                }
                formatted_grid.at(i * width + j) = raw_map.at(raw_y * true_width + raw_x);
            }
            raw_x = 0;
            m = 0;
        }
        return formatted_grid;
    }

    /**
     * @brief Calculates the width and length for the formatted grid.
     *
     * @param size_direction Specify wether the width or the length is needed.
     * @return Returns a 32-bit integer with the width or length measured in pixels.
     */
    uint32_t show_formatted_dimensions(const std::string &size_direction)
    {
        if (size_direction == "width")
        {
            uint32_t width = (((uint32_t)x_handle.at((size_t)std::distance(x_handle.begin(), std::max_element(x_handle.begin(), x_handle.end()))) - (uint32_t)x_handle.at((size_t)std::distance(x_handle.begin(), std::min_element(x_handle.begin(), x_handle.end())))) / x_step.at((size_t)std::distance(x_step.begin(), std::min_element(x_step.begin(), x_step.end())))) + 1;

            if (width % 4 != 0)
            {
                width = width + 4 - (width % 4);
            }
            return width;
        }
        else if (size_direction == "length")
        {
            uint32_t length = (((uint32_t)y_handle.at((size_t)std::distance(y_handle.begin(), std::max_element(y_handle.begin(), y_handle.end()))) - (uint32_t)y_handle.at((size_t)std::distance(y_handle.begin(), std::min_element(y_handle.begin(), y_handle.end())))) / y_step.at((size_t)std::distance(y_step.begin(), std::min_element(y_step.begin(), y_step.end())))) + 1;

            if (length % 4 != 0)
            {
                length = length + 4 - (length % 4);
            }
            return length;
        }
        else
            throw std::invalid_argument("Can't access requested dimension");
    }

private:
    uint32_t true_width = 0;
    uint32_t true_length = 0;
    std::vector<double> x_handle;
    std::vector<double> y_handle;
    std::vector<uint32_t> x_step;
    std::vector<uint32_t> y_step;
    std::vector<double> raw_map;
};

//                                            End class data_map                                            //
//==========================================================================================================//
//                                           Begin class BmpHeader                                          //

/**
 * @brief Class to create BMP file metadata. Build based on a tutorial from https://dev.to/muiz6/c-how-to-write-a-bitmap-image-from-scratch-1k6m (No lines were copied, but it was used as a guide for the information required)
 */
class BmpHeader
{
public:
    /**
     * @brief Construct the header for the BMP file. Only width and height get modified depending on the formatted data map object.
     *
     * @param width Calculated width of the formatted grid.
     * @param length Calculated height of the formatted grid.
     */
    BmpHeader(const uint64_t &width, const uint64_t &length)
    {
        if (width > INT32_MAX or length > INT32_MAX)
        {
            std::cout << "Map dimensions might result in unexpected behavior. Get formatted map with external argument";
            exit(0);
        }
        else if (width % 4 != 0 or length % 4 != 0)
        {
            std::cout << "Dimensions are not allowed for BMP";
            exit(0);
        }
        uint32_t size_of_map = static_cast<uint32_t>(width * length);
        sizeOfBitmapFile += (size_of_map * 4);
    }

    /**
     * @brief Writes the bitmap header in the file.
     *
     * @param outputbm Reference to the created BMP file.
     *
     */
    void write_header(std::ofstream &outputbm)
    {
        outputbm.write(this->bitmapSignatureBytes, 2);
        outputbm.write((char *)&this->sizeOfBitmapFile, sizeof(uint32_t));
        outputbm.write((char *)&this->reservedBytes, sizeof(uint32_t));
        outputbm.write((char *)&this->pixelDataOffset, sizeof(uint32_t));
    }

private:
    char bitmapSignatureBytes[2] = {'B', 'M'};
    uint32_t sizeOfBitmapFile = 54;
    uint32_t reservedBytes = 0;
    uint32_t pixelDataOffset = 54;
};

//                                            End class BmpHeader                                            //
//===========================================================================================================//
//                                         Begin class BmpInfoHeader                                         //

/**
 * @brief Class to create BMP file metadata. Build based on a tutorial from https://dev.to/muiz6/c-how-to-write-a-bitmap-image-from-scratch-1k6m (No lines were copied, but it was used as a guide for the information required)
 */
class BmpInfoHeader
{
public:
    /**
     * @brief Construct a new Bmp Info Header for the BMP file. Only width and height are modified with the computed width and height.
     *
     * @param formatted_width Computed width of formatted grid in pixels
     * @param formatted_length Computed height of formatted height in pixels
     */
    BmpInfoHeader(const uint64_t &formatted_width, const uint64_t &formatted_length)
    {
        if (formatted_width > INT32_MAX or formatted_length > INT32_MAX)
        {
            std::cout << "Map dimensions might result in unexpected behavior. Get formatted map with external argument";
            exit(0);
        }
        width = static_cast<int32_t>(formatted_width);
        height = static_cast<int32_t>(formatted_length);
        int32_t a = width;
        int32_t b = height;
        int32_t r;
        do
        {
            r = a % b;
            a = b;
            b = r;
        } while (r);

        horizontalResolution = horizontalResolution * (width / a);
        verticalResolution = verticalResolution * (height / a);
    }

    /**
     * @brief Write the Info Header in to the previously created BMP file.
     *
     * @param outputbm Reference to the BMP file.
     */
    void write_infoheader(std::ofstream &outputbm)
    {
        outputbm.write((char *)&this->sizeOfThisHeader, sizeof(uint32_t));
        outputbm.write((char *)&this->width, sizeof(int32_t));
        outputbm.write((char *)&this->height, sizeof(int32_t));
        outputbm.write((char *)&this->numberOfColorPlanes, sizeof(uint16_t));
        outputbm.write((char *)&this->colorDepth, sizeof(uint16_t));
        outputbm.write((char *)&this->compressionMethod, sizeof(uint32_t));
        outputbm.write((char *)&this->rawBitmapDataSize, sizeof(uint32_t));
        outputbm.write((char *)&this->horizontalResolution, sizeof(int32_t));
        outputbm.write((char *)&this->verticalResolution, sizeof(int32_t));
        outputbm.write((char *)&this->colorTableEntries, sizeof(uint32_t));
        outputbm.write((char *)&this->importantColors, sizeof(uint32_t));
    }

private:
    uint32_t sizeOfThisHeader = 40;
    int32_t width = 0;
    int32_t height = 0;
    uint16_t numberOfColorPlanes = 1;
    uint16_t colorDepth = 24;
    uint32_t compressionMethod = 0;
    uint32_t rawBitmapDataSize = 0;
    int32_t horizontalResolution = 1000;
    int32_t verticalResolution = 1000;
    uint32_t colorTableEntries = 0;
    uint32_t importantColors = 0;
};

//                                          End class BmpInfoHeader                                           //
//============================================================================================================//
//                                          Begin output functions                                            //

/**
 * @brief Creates a file and writes the matrix with format. Applies for both the raw and the bitmap matrix.
 *
 * @param map Matrix with the intensity values to be in the output.
 * @param width Width of the figure, comes from the class data_map functions.
 * @param length Height of the figure, comes from the class data_map functions.
 * @param output_filename Title of the output file.
 */
void external_plot(const std::vector<double> &map, const uint64_t &width, const uint64_t &length, std::string &output_filename)
{
    if ((length * width) != map.size())
    {
        std::cout << "Dimensions and map do not coincide";
        exit(0);
    }
    std::string filename = output_filename + ".txt";
    std::ofstream output(filename);
    if (!output.is_open())
    {
        std::cout << "Error opening the file " << filename << "!";
        exit(0);
    }

    for (uint64_t i = 0; i < length; i++)
    {
        for (uint64_t j = 0; j < width; j++)
        {

            output << std::setw(10) << std::setprecision(6);
            output << map.at(i * width + j);
            if (j != width)
                output << ' ';
        }
        output << '\n';
    }
    output.close();
    std::cout << "Created file: " << filename << " with matrix to plot image externally." << '\n';
}

/**
 * @brief Creates two files one for the x axis and one for the y axis values for the raw map.
 * Used to plot in other programming languages or software.
 *
 * @param x Vector with the x axis points to be printed. Comes from function of data_map class.
 * @param y Vector with the y axis points to be printed. Comes from function of data_map class.
 * @param output_filename Title of the output file.
 */
void external_plot_axis(std::vector<double> &x, std::vector<double> &y, std::string &output_filename)
{
    std::string filename1 = output_filename + "-x-axis-handles.txt";
    std::string filename2 = output_filename + "-y-axis-handles.txt";
    std::ofstream output1(filename1);
    if (!output1.is_open())
    {
        std::cout << "Error opening the file: " << filename1 << "!";
    }

    for (uint64_t i = 0; i < x.size(); i++)
    {
        output1 << x.at(i) << '\n';
    }

    output1.close();
    std::cout << "Created file:" << filename1 << " with x-axis handles to plot image externally." << '\n';

    std::ofstream output2(filename2);
    if (!output2.is_open())
    {
        std::cout << "Error opening the file: " << filename1 << "!";
    }
    for (uint64_t i = 0; i < y.size(); i++)
    {
        output2 << y.at(i) << '\n';
    }
    output2.close();

    std::cout << "Created file:" << filename1 << " with y-axis handles to plot image externally." << '\n';
}

/**
 * @brief Creates a binary BMP file. Build based on a tutorial from https://dev.to/muiz6/c-how-to-write-a-bitmap-image-from-scratch-1k6m (No lines were copied, but it was used as a guide for the information required)
 *
 * @param intensity Map values to set the colour of the image.
 * @param width Calculated width in pixels from the class data_map function.
 * @param length Calculated height in pixels from the class data_map function.
 * @param output_filename Title of the BMP file.
 */
void build_bitmap(std::vector<double> &intensity, const uint64_t &width, const uint64_t &length, std::string &output_filename)
{
    std::string filename = output_filename + ".bmp";
    BmpHeader header(width, length);
    BmpInfoHeader info_header(width, length);
    std::ofstream outputbm(filename, std::ios::binary);
    if (!outputbm.is_open())
    {
        std::cout << "Error creating BMP File";
        exit(0);
    }
    if (width % 4 != 0 or length % 4 != 0)
    {
        std::cout << "Dimensions are not suitable for bitmap." << '\n';
        exit(0);
    }

    header.write_header(outputbm);
    info_header.write_infoheader(outputbm);
    for (uint64_t i = 0; i < intensity.size(); i++)
    {
        if (intensity[i] > 1)
        {
            std::cout << "Error: Intensity map not suitable for bitmap build";
            exit(0);
        }
        uint8_t blue = static_cast<uint8_t>(75 * intensity[i] / 0.8);
        outputbm.write((char *)&blue, sizeof(uint8_t));
        uint8_t green = static_cast<uint8_t>(145 * intensity[i] / 0.3);
        outputbm.write((char *)&green, sizeof(uint8_t));
        uint8_t red = static_cast<uint8_t>(250 * intensity[i] / 0.2);
        outputbm.write((char *)&red, sizeof(uint8_t));
    }
    outputbm.close();

    std::cout << "Successfully created: " + filename << '\n';
}

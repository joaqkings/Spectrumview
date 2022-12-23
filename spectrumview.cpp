/**
 * @file spectrumview.cpp
 * @author Joaquin Reyes (reyesgoj@mcmaster.ca)
 * @brief A program using the header file spectrum_map to get either files to plot spatially resolved spectra as a BMP file in other languages or software
 * @version 0.1
 * @date 2022-12-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <map>
#include <tuple>
#include <set>
#include "spectrum_map.hpp"
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    try
    {
        if (argc == 1)
        {
            std::cout << "Welcome to spectrumview!" << '\n'
                      << '\n'
                      << "To create raw files and bitmap syntax is:" << '\n'
                      << "\n./spectrumview + 'Path to directory' + Format + Intensity mode + Output file name + Energy of interest" << '\n'
                      << "\nFormat is: [All] to get all files, [Raw] to get raw map file and handles, [Grid] to get grid file and [bmp] to get bitmap." << '\n'
                      << "\nIntensity mode is: [Integrated] to add the intensities of a range of channels, its followed by the amount of energy channels or [Interpolated] to get the intensity at a specific energy." << '\n'
                      << "\nOutput file name will be modified with details of the energy and type of output" << '\n'
                      << "\nThe final value correspond to the energy of interest. " << '\n'
                      << "\nExample:" << '\n'
                      << "\n./spectrumview 'C:/Users/Scientist/EELS' All Integrated 2 EELS_Spectrum_map 0.035" << '\n';
        }
        else if (argc > 1 and argc < 6)
        {
            std::cout << "Not enough arguments to run the program" << '\n';
            exit(0);
        }
        else if (argc == 6)
        {

            if (!std::strcmp(argv[3], "integrated"))
            {
                throw std::invalid_argument("Input in terminal has unrecognized intensity mode! Can only be interpolated");
            }
            std::string energy = argv[5];
            for (std::string::iterator c = energy.begin(); c < energy.end(); c++)
            {
                if (!isdigit(*c))
                {
                    if (*c != '.')
                        throw std::invalid_argument("Energy must be a float or an integer");
                }
            }
            std::vector<fs::path> myFiles = opendirectory(argv[1]);
            std::map<std::tuple<double, double>, double> mapfilling;
            std::set<std::tuple<double, double>> coordinate_list;

            for (std::vector<fs::path>::iterator i = myFiles.begin(); i < myFiles.end(); i++)
            {
                spectrum current_spectrum(*i);
                double extracted_intensity = current_spectrum.interpolated_intensity(std::stod(argv[5]));
                std::tuple<double, double> coordinates = std::make_tuple(current_spectrum.show_position("x"), current_spectrum.show_position("y"));
                if (mapfilling.contains(coordinates))
                {
                    std::cout << "Two files found for the same position. Make sure directory only has one file per position.";
                    exit(0);
                }
                else
                {
                    coordinate_list.insert(coordinates);
                    mapfilling[coordinates] = extracted_intensity;
                }
            }

            std::string project_title = argv[4];
            data_map spectra_map(coordinate_list, mapfilling);

            if (!std::strcmp(argv[2], "raw") or !std::strcmp(argv[2], "all"))
            {
                std::string raw_title = project_title + "raw";
                std::vector<double> map = spectra_map.show_raw();
                uint32_t width = spectra_map.show_dimensions("width");
                std::cout << "Raw width is: " << width << '\n';
                uint32_t height = spectra_map.show_dimensions("length");
                std::cout << "Raw height is: " << height << '\n';
                external_plot(map, width, height, raw_title);
                std::vector<double> x = spectra_map.show_axis("x");
                std::vector<double> y = spectra_map.show_axis("y");
                external_plot_axis(x, y, raw_title);
            }
            if (!std::strcmp(argv[2], "grid") or !std::strcmp(argv[2], "all") or !std::strcmp(argv[2], "bmp"))
            {
                std::string grid_title = project_title + "grid";
                std::vector<double> formatted_map = spectra_map.show_formatted_grid();
                uint32_t width = spectra_map.show_formatted_dimensions("width");
                std::cout << "Formatted width is: " << width << '\n';
                uint32_t height = spectra_map.show_formatted_dimensions("length");
                std::cout << "Formatted height is:" << height << '\n';
                if (!std::strcmp(argv[2], "grid") or !std::strcmp(argv[2], "all"))
                    external_plot(formatted_map, width, height, grid_title);
                if (!std::strcmp(argv[2], "bmp") or !std::strcmp(argv[2], "all"))
                    build_bitmap(formatted_map, width, height, project_title);
            }
            else
                throw std::invalid_argument("Specified format not identified. Allowed format is: all, grid, raw, bmp");
        }
        else if (argc == 7 and !std::strcmp(argv[3], "integrated"))
        {

            std::string energy = argv[6];
            for (std::string::iterator c = energy.begin(); c < energy.end(); c++)
            {
                if (!isdigit(*c))
                {
                    if (*c != '.')
                        throw std::invalid_argument("Energy must be a float or an integer");
                }
            }
            std::string channel = argv[4];
            for (std::string::iterator c = channel.begin(); c < channel.end(); c++)
            {
                if (!isdigit(*c))
                {
                    throw std::invalid_argument("channel must be an integer");
                }
            }
            std::vector<fs::path> myFiles = opendirectory(argv[1]);
            std::map<std::tuple<double, double>, double> mapfilling;
            std::set<std::tuple<double, double>> coordinate_list;
            for (std::vector<fs::path>::iterator i = myFiles.begin(); i < myFiles.end(); i++)
            {
                spectrum current_spectrum(*i);
                double extracted_intensity = current_spectrum.integrated_intensity(std::stod(argv[6]), std::stoull(argv[4]));
                std::tuple<double, double> coordinates = std::make_tuple(current_spectrum.show_position("x"), current_spectrum.show_position("y"));
                if (mapfilling.contains(coordinates))
                {
                    std::cout << "Two files found for the same position. Make sure directory only has one file per position.";
                    exit(0);
                }
                else
                {
                    coordinate_list.insert(coordinates);
                    mapfilling[coordinates] = extracted_intensity;
                }
            }

            std::string project_title = argv[5];
            data_map spectra_map(coordinate_list, mapfilling);

            if (!std::strcmp(argv[2], "raw") or !std::strcmp(argv[2], "all"))
            {
                std::string raw_title = project_title + "raw";
                std::vector<double> map = spectra_map.show_raw();
                uint64_t width = spectra_map.show_dimensions("width");
                std::cout << "Raw width is: " << width << '\n';
                uint64_t height = spectra_map.show_dimensions("length");
                std::cout << "Raw height is: " << height << '\n';
                external_plot(map, width, height, project_title);
                std::vector<double> x = spectra_map.show_axis("x");
                std::vector<double> y = spectra_map.show_axis("y");
                external_plot_axis(x, y, project_title);
            }

            if (!std::strcmp(argv[2], "grid") or !std::strcmp(argv[2], "all") or !std::strcmp(argv[2], "bmp"))
            {
                std::string grid_title = project_title + "grid";
                std::vector<double> formatted_map = spectra_map.show_formatted_grid();
                uint64_t width = spectra_map.show_formatted_dimensions("width");
                std::cout << "Formatted width is: " << width << '\n';
                uint64_t height = spectra_map.show_formatted_dimensions("length");
                std::cout << "Formatted height is:" << height << '\n';
                if (!std::strcmp(argv[2], "grid") or !std::strcmp(argv[2], "all"))
                    external_plot(formatted_map, width, height, project_title);
                if (!std::strcmp(argv[2], "bmp") or !std::strcmp(argv[2], "all"))
                    build_bitmap(formatted_map, width, height, project_title);
            }
        }
    }
    catch (std::invalid_argument const &e)
    {
        std::cout << e.what() << '\n';
    }
}

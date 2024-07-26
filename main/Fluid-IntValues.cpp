#define _POSIX_SOURCE
 
#include <stdlib.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string>
#include <cstring>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <conio.h> 
#include <string>
#include <chrono>
#include <thread>
#include <cmath>
#include <unordered_map>

#include "lodepng.h"

const int valuePipeVolume = 100;

struct segment{
    int flagEmpty;
    int amountFluidChange;
    int amountFluid;
    int amountFluidProcessed;
    std::unordered_map<int, int> mapFlow;
    std::unordered_map<int, int> mapMovement;
    std::vector<int> arrayPipes;
};

struct pipe{
    int indexSegment;
    int indexConnections[4];
};

std::vector<unsigned char> arrayImagePixels;
std::vector<int> arrayLinksPixelToPipe;

char stringGraphics[100000];
int indexGraphics;
char c;

void fAddToBuffer(const char* sAdd) {
    int iA = 0;
    while (sAdd[iA]) {
        stringGraphics[indexGraphics++] = sAdd[iA++];
    }
}

int main() {
    std::string sFile = "FluidSystem.png";
    int parameterArrows = 0;
    int parameterBoxes = 0;
    printf("How to display fluid flows: 0 - arrows, 1 - fluid moved, 2 - desired flow...\n");
    scanf("%i", &parameterArrows);

    printf("Reading pipe system image...\n");
    unsigned errorPNG;
    unsigned sizeImageWidth;
    unsigned sizeImageHeight;
    unsigned vPixelAmount;
    errorPNG = lodepng::decode(arrayImagePixels, sizeImageWidth, sizeImageHeight, sFile.c_str());
    if (errorPNG) {
        printf("Image Read Error %i: %s\n", errorPNG, lodepng_error_text(errorPNG));
        return 0;
    }
    printf("Height = % i | Width = %i\n", sizeImageHeight, sizeImageWidth);
    vPixelAmount = sizeImageHeight * sizeImageWidth;

    printf("Constructing system via image...\n");
    arrayLinksPixelToPipe.resize(vPixelAmount);
    std::vector<pipe> arrayPipes;
    std::vector<segment> arraySegments;

    unsigned indexImage = 0;
    unsigned indexPixel = 0;
    int indexPipe = 0;
    for (int indexRow = 0; indexRow < sizeImageHeight; indexRow++) {
        for (int indexColumn = 0; indexColumn < sizeImageWidth; indexColumn++) {
            int flagPipe = 0;
            int valueChange = 0;
            if (arrayImagePixels[indexPixel + 0]) {
                // Output
                valueChange = arrayImagePixels[indexPixel + 0] / -4;
                flagPipe = 1;
            } else if (arrayImagePixels[indexPixel + 1]) {
                // Input
                valueChange = arrayImagePixels[indexPixel + 1] / 4;
                flagPipe = 1;
            } else if (arrayImagePixels[indexPixel + 2]) {
                // Pipe
                flagPipe = 1;
            } else {
                arrayLinksPixelToPipe[indexImage] = -1;
            }

            if (flagPipe) {
                arraySegments.push_back(segment());
                arrayPipes.push_back(pipe());

                arrayLinksPixelToPipe[indexImage] = indexPipe;
                arrayPipes[indexPipe].indexSegment = indexPipe;

                arraySegments[indexPipe].arrayPipes = {(int)indexPipe};
                arraySegments[indexPipe].amountFluidChange = valueChange;

                indexPipe++;
            }
            indexPixel += 4;
            indexImage++;
        }
    }

    printf("Adding segment connections...\n");
    indexImage = 0;
    int arrayNeighbors[4];
    for (int indexRow = 0; indexRow < sizeImageHeight; indexRow++) {
        for (int indexColumn = 0; indexColumn < sizeImageWidth; indexColumn++) {
            indexPipe = arrayLinksPixelToPipe[indexImage];
            if (indexPipe != -1) {

                arrayNeighbors[0] = (indexImage >= sizeImageWidth) ? indexImage - sizeImageWidth : -1;
                arrayNeighbors[1] = (indexImage % sizeImageWidth != 0) ? indexImage - 1 : -1;
                arrayNeighbors[2] = ((indexImage + 1) % sizeImageWidth != 0) ? indexImage + 1 : -1;
                arrayNeighbors[3] = (indexImage + sizeImageWidth < vPixelAmount) ? indexImage + sizeImageWidth : -1;

                for (int indexNeighbor = 0; indexNeighbor < 4; indexNeighbor++) {
                    if (arrayNeighbors[indexNeighbor] != -1 && arrayLinksPixelToPipe[arrayNeighbors[indexNeighbor]] != -1) {
                        arraySegments[indexPipe].mapFlow[arrayLinksPixelToPipe[arrayNeighbors[indexNeighbor]]] = 0;
                        arraySegments[arrayLinksPixelToPipe[arrayNeighbors[indexNeighbor]]].mapFlow[indexPipe] = 0;
                    }
                    arrayPipes[indexPipe].indexConnections[indexNeighbor] = arrayNeighbors[indexNeighbor];
                }
            }
            indexImage++;
        }
    }

    printf("Prapairing simlation...\n");
    segment *segmentCurrent;
    int amountFluidMove;
    int indexSegment;
    int amountSegments = arraySegments.size();
    int valueMaxFlow = valuePipeVolume / 2;

    int vSysIn;
    int vSysOut;
    int vSysBalance = 0;
    int amountOptimized = 1;
    int counterTick = 0;

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    while (true) {
        // GRAPHICS ====================================================================================================================================
        // PLEASE LEAVE IT AS IT IS, I WILL JUST MAY WRITE PROPER INTERFACE ============================================================================
        indexImage = 0;
        indexGraphics = 0;
        fAddToBuffer("\x1B[2J\x1B[H");
        for (int indexRow = 0; indexRow < sizeImageHeight; indexRow++) {
            for (int indexSubRow = 0; indexSubRow < 3; indexSubRow++) {
                for (int indexColumn = 0; indexColumn < sizeImageWidth; indexColumn++) {
                    for (int indexSubColumn = 0; indexSubColumn < 3; indexSubColumn++) {
                        int vType = 0;
                        
                        indexPipe = arrayLinksPixelToPipe[indexImage];
                        if (indexPipe == -1) {
                            fAddToBuffer("  ");
                            continue;
                        }
                        indexSegment = arrayPipes[indexPipe].indexSegment;

                        int valueDirection = -1;
                        if (indexSubColumn == 1) {
                            //
                            if (indexSubRow == 1) {
                                // Central segment
                                vType = 1;
                            } else {
                                if (indexSubRow == 0) {
                                    // Direction Up
                                    valueDirection = 0;
                                } else {
                                    // Direction Down
                                    valueDirection = 3;
                                }
                            }
                        } else if (indexSubRow == 1) {
                            if (indexSubColumn == 1) {
                                // Central segment
                                vType = 1;
                            } else {
                                if (indexSubColumn == 0) {
                                    // Direction Left
                                    valueDirection = 1;
                                } else {
                                    // Direction Right
                                    valueDirection = 2;
                                }
                            }
                        }

                        if (valueDirection != -1) {
                            int indexNeighbor = arrayPipes[indexPipe].indexConnections[valueDirection];
                            if (indexNeighbor != -1) {
                                indexNeighbor = arrayLinksPixelToPipe[indexNeighbor];
                            }
                            if (indexNeighbor != -1) {
                                if (arrayPipes[indexNeighbor].indexSegment == indexSegment) {
                                    // Pipe is in the same segment
                                    vType = 1;
                                } else {
                                    // Pipe is in the different segment
                                    vType = 2;
                                }
                            } else {
                                vType = 0;
                            }
                        }

                        int vRed = 0;
                        int vGreen = 0;
                        int vBlue = 0;
                        if (vType == 1) {
                            if (arraySegments[indexSegment].amountFluidChange > 0) {
                                // Making color for segment with fluid produciton
                                if ((int)arraySegments[indexSegment].amountFluidProcessed == (int)arraySegments[indexSegment].amountFluidChange) {
                                    vGreen = 255;
                                } else {
                                    vGreen = std::min(255, (int)(arraySegments[indexSegment].amountFluidProcessed * 4) + 35);
                                }
                            } else if (arraySegments[indexSegment].amountFluidChange < 0) {
                                // Making color for segment with fluid consumption
                                if ((int)arraySegments[indexSegment].amountFluidProcessed == (int)arraySegments[indexSegment].amountFluidChange) {
                                    vRed = 255;
                                } else {
                                    vRed = std::min(255, (int)(arraySegments[indexSegment].amountFluidProcessed * -4) + 35);
                                }
                            } else {
                                // Making color for pipe
                                vRed = 25;
                                vBlue = std::min(((arraySegments[indexSegment].amountFluid * 230) / (int)arraySegments[indexSegment].arrayPipes.size() / valuePipeVolume), 230) + 25;
                                vGreen = vBlue;
                            }
                            fAddToBuffer(("\033[38;2;" + std::to_string(vRed) + ';' + std::to_string(vGreen) + ';' + std::to_string(vBlue) + 'm').c_str());

                            stringGraphics[indexGraphics++] = 219;
                            stringGraphics[indexGraphics++] = 219;
                          
                        } else if (vType == 2) {
                            // Drawing flow arrows
                            int indexNeighbor = arrayLinksPixelToPipe[arrayPipes[indexPipe].indexConnections[valueDirection]];
                            indexNeighbor = arrayPipes[indexNeighbor].indexSegment;
                            int flagIn = 0;
                            if (arraySegments[indexSegment].mapMovement[indexNeighbor] >= 0.0f) {
                                vGreen = (arraySegments[indexSegment].mapMovement[indexNeighbor] * 255) / valueMaxFlow;
                                vGreen = std::min(255, vGreen);
                                flagIn = 1;
                            } else if (arraySegments[indexSegment].mapMovement[indexNeighbor] < 0.0f) {
                                vRed = (-arraySegments[indexSegment].mapMovement[indexNeighbor] * 255) / valueMaxFlow;
                                vRed = std::min(255, vRed);
                            }

                            fAddToBuffer(("\033[38;2;" + std::to_string(vRed) + ';' + std::to_string(vGreen) + ';' + std::to_string(vBlue) + 'm').c_str());
                            
                            if (parameterArrows == 1) {
                                if (std::abs(arraySegments[indexSegment].mapMovement[indexNeighbor]) < 10) fAddToBuffer("0");
                             fAddToBuffer(std::to_string(std::abs(arraySegments[indexSegment].mapMovement[indexNeighbor])).c_str());
                            } else if (parameterArrows == 2) {
                                if (std::abs(arraySegments[indexSegment].mapFlow[indexNeighbor]) < 10) fAddToBuffer("0");
                                fAddToBuffer(std::to_string(std::abs(arraySegments[indexSegment].mapFlow[indexNeighbor])).c_str());
                            } else {
                                if (flagIn && valueDirection == 2 || !flagIn && valueDirection == 1){
                                    fAddToBuffer(">>");
                                } else if (flagIn && valueDirection == 1 || !flagIn && valueDirection == 2){
                                    fAddToBuffer("<<");
                                } else if (flagIn && valueDirection == 0 || !flagIn && valueDirection == 3) {
                                    fAddToBuffer("^^");
                                } else {
                                    fAddToBuffer("vv");
                                }
                            }
                        } else {
                            fAddToBuffer("  ");
                        }
                    }
                    indexImage++;
                }

                stringGraphics[indexGraphics++] = '\n';
                if (indexSubRow != 2) {
                    indexImage -= sizeImageWidth;
                }
            }
        }
        // ==================================================================================================================================================

        vSysIn = 0;
        vSysOut = 0;

        // Taking Fluids
        for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
            segmentCurrent = &arraySegments[indexSegment];
            if (segmentCurrent->flagEmpty) continue;

            // Just add max possible amount of fluid to a segment. Processed value and system balance are purely for debugging.
            if (segmentCurrent->amountFluidChange < 0) {
                amountFluidMove = std::min(segmentCurrent->amountFluid, -segmentCurrent->amountFluidChange);
                segmentCurrent->amountFluid -= amountFluidMove;
                segmentCurrent->amountFluidProcessed = -amountFluidMove;
                vSysOut += amountFluidMove;
            }
        }

        // Adding Fluids
        for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
            segmentCurrent = &arraySegments[indexSegment];
            if (segmentCurrent->flagEmpty) continue;

            // Just remove max possible amount of fluid to a segment. Processed value and system balance are purely for debugging.
            if (segmentCurrent->amountFluidChange > 0) {
                amountFluidMove = std::min(valuePipeVolume - segmentCurrent->amountFluid, segmentCurrent->amountFluidChange);
                segmentCurrent->amountFluid += amountFluidMove;
                vSysIn += amountFluidMove;
                segmentCurrent->amountFluidProcessed = amountFluidMove;
            }
        }

        // Add fluid pressure
        for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
            segmentCurrent = &arraySegments[indexSegment];
            if (segmentCurrent->flagEmpty) continue;

            // Each segment's fluid just wants to equally be distributed to neighbors.
            // The desired flow value is accumulative, so it will quickly sum up and go to the possible max flow (half of a pipe size per tick)
            // When flow is formed, the fluid between neighbor segments will prevent flow values from going into infinity.
            int valueChange = ((segmentCurrent->amountFluid) / (int)segmentCurrent->arrayPipes.size()) / 3;
            for (auto &segmentDestination : segmentCurrent->mapFlow) {
                segmentDestination.second += valueChange;
                arraySegments[segmentDestination.first].mapFlow[indexSegment] -= valueChange;
            }
        }


        // Creating identical temporal movement map
        // Just copy Flow into Movement (do it in one command please)
        // We will adjust movement so segments will not overfill or go below 0 in fluid content
        for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
            segmentCurrent = &arraySegments[indexSegment];
            for (auto &infoToCombine : segmentCurrent->mapFlow) {
                infoToCombine.second = std::min(infoToCombine.second, valueMaxFlow);
                infoToCombine.second = std::max(infoToCombine.second, -valueMaxFlow);
                segmentCurrent->mapMovement[infoToCombine.first] = infoToCombine.second; 
            }
        }

        // Movement adjusting for out-flows
        for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
            segmentCurrent = &arraySegments[indexSegment];
            if (segmentCurrent->flagEmpty) continue;

            // Get amount of neighbors to which segemnt wants to move fluid out, and the volume of fluid.
            int valueOutputs = 0;
            for (auto &infoToCombine : segmentCurrent->mapMovement) {
                if (infoToCombine.second > 0) {
                    valueOutputs += infoToCombine.second;
                }
            }

            // If flow out is bigger than current segment fluid content, adjust all outputs proportianally to maximum possible flow out.
            int valueLimit = std::min(segmentCurrent->amountFluid, valueMaxFlow);
            int valueCorrection = 0;
            if (valueOutputs > valueLimit) {
                for (auto &infoToCombine : segmentCurrent->mapMovement) {
                    if (infoToCombine.second > 0) {
                        int valuePartition = infoToCombine.second * valueLimit;
                        infoToCombine.second = valuePartition / valueOutputs;
                        int valueMod = valuePartition % valueOutputs;
                        if (valueMod > 0) {
                            valueCorrection += valueMod;
                            if (valueCorrection > 0) {
                                infoToCombine.second++;
                                valueCorrection -= valueOutputs;
                            }
                        }
                        arraySegments[infoToCombine.first].mapMovement[indexSegment] = -infoToCombine.second;
                    }
                }
            }
        }

        // Movement adjusting for in-flows
        for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
            segmentCurrent = &arraySegments[indexSegment];
            if (segmentCurrent->flagEmpty) continue;

            // Get amount of neighbors who want to move fluid in, and the volume of fluid.
            int valueInputs = 0;
            for (auto &infoToCombine : segmentCurrent->mapMovement) {
                if (infoToCombine.second < 0) {
                    valueInputs -= infoToCombine.second;
                }
            }

            // If flow in is bigger than current segment capacity, adjust all inputs proportianally to maximum possible flow in.
            int valueLimit = std::min(valuePipeVolume * (int)segmentCurrent->arrayPipes.size() - segmentCurrent->amountFluid, valueMaxFlow);
            int valueCorrection = 0;
            if (valueInputs > valueLimit) {
                for (auto &infoToCombine : segmentCurrent->mapMovement) {
                    if (infoToCombine.second < 0) {
                        int valuePartition = (-infoToCombine.second) * valueLimit;
                        infoToCombine.second = -(valuePartition / valueInputs);
                        int valueMod = valuePartition % valueInputs;
                        if (valueMod > 0) {
                            valueCorrection += valueMod;
                            if (valueCorrection > 0) {
                                infoToCombine.second--;
                                valueCorrection -= valueInputs;
                            }
                        }
                        arraySegments[infoToCombine.first].mapMovement[indexSegment] = -infoToCombine.second;
                    }
                }
            }
        }

        //Fluid moving
        for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
            segmentCurrent = &arraySegments[indexSegment];
            if (segmentCurrent->flagEmpty) continue;

            // For each segment perform it's move map but only for flow inside, as for another segment it is flow outside.
            for (auto &infoToCombine : segmentCurrent->mapMovement) {
                if (infoToCombine.second > 0) {
                    segmentCurrent->amountFluid -= infoToCombine.second;
                    arraySegments[infoToCombine.first].amountFluid += infoToCombine.second;
                }
            }
        }

        // MORE GRAPHICS =========================================================
        vSysBalance += vSysIn - vSysOut;
        fAddToBuffer(("\033[38;2;255;255;255mAdded = " + std::to_string(vSysIn) + " | Removed = " + std::to_string(vSysOut) + " | Fluid = " + std::to_string(vSysBalance) + " | Tick " + std::to_string(++counterTick) + '\n').c_str());
        stringGraphics[indexGraphics++] = '\n';
        stringGraphics[indexGraphics++] = 0;
        std::cout << stringGraphics << std::flush;
        /*scanf("%s", &c);
        printf("|%c|\n", c);*/
        scanf("%c", &c);

        // Segment optimization
        if (amountOptimized) {
            amountOptimized = 0;
            
            for (indexSegment = 0; indexSegment < amountSegments; indexSegment++) {
                segment *segmentThis = &arraySegments[indexSegment];
                if (segmentThis->flagEmpty) continue;

                for (auto infoToCombine : segmentThis->mapFlow) {
                    segment *segmentCombine = &arraySegments[infoToCombine.first];
                    if (segmentCombine->flagEmpty) continue;

                    int flagCombine = 0;
                    // Case 1: Basic straight line segment combining
                    if (segmentThis->mapFlow.size() <= 2 && segmentCombine->mapFlow.size() <= 2) {
                        if (segmentThis->amountFluidChange == 0 && segmentCombine->amountFluidChange == 0) {
                            flagCombine = 1;
                        }
                    }

                    // I NEED *YOU* TO TELL ME MORE COMBINE CASES

                    // Combine segments if possible
                    if (flagCombine) {
                        // Mark segment as inactive, later recombine all segment array
                        segmentCombine->flagEmpty = 1;

                        // Combining movement map
                        segmentThis->mapMovement.erase(infoToCombine.first);
                        for (auto infoFromCombine : segmentCombine->mapMovement) {
                            if (infoFromCombine.first != indexSegment) {
                                segmentThis->mapMovement.insert(infoFromCombine);

                                arraySegments[infoFromCombine.first].mapMovement[indexSegment] = arraySegments[infoFromCombine.first].mapMovement[infoToCombine.first];
                                arraySegments[infoFromCombine.first].mapMovement.erase(infoToCombine.first);
                            }
                        }

                        // Combining flow map
                        segmentThis->mapFlow.erase(infoToCombine.first);
                        for (auto infoFromCombine : segmentCombine->mapFlow) {
                            if (infoFromCombine.first != indexSegment) {
                                segmentThis->mapFlow.insert(infoFromCombine);

                                arraySegments[infoFromCombine.first].mapFlow[indexSegment] = arraySegments[infoFromCombine.first].mapFlow[infoToCombine.first];
                                arraySegments[infoFromCombine.first].mapFlow.erase(infoToCombine.first);
                            }
                        }

                        // Combining pipes
                        segmentThis->arrayPipes.insert(segmentThis->arrayPipes.end(), segmentCombine->arrayPipes.begin(), segmentCombine->arrayPipes.end());

                        // Combining fluids
                        segmentThis->amountFluid += segmentCombine->amountFluid;
                        segmentThis->amountFluidProcessed += segmentCombine->amountFluidProcessed;

                        // Rewrite pipe to segment connections
                        for (int indexPipe : segmentCombine->arrayPipes) {
                            arrayPipes[indexPipe].indexSegment = indexSegment;
                        }

                        // ===
                        amountOptimized++;
                        break;
                    }
                }
            }
        }
        // End of one tick
    }

    return 0;
}

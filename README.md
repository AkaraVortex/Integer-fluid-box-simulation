This is my fan project sparked by fluid changes announced to Factorio 2.0

Main purpose was to create intuitive system where flow is constant through any amount of segment.
Main drawbacks of 1.1 system were unpredictable fluid behavior on junctions and weird max flowrates based on length of a pipe, and, of course, it was too limiting for 2.0 standards and was hard to

My system provides following:
- Presudo-realistic behaviour, fluid on junctions spreads equally.
- Constant and fixed flowrate for any pipe section (between junctions).
- Integer values, meaning faster compute times, less memory usage and not fluid losses due to float variables value losses.
- Ability to merge multiple fluid boxes into single segment, not affecting whole system's behaviour and max flowrates across any possible path. That allows for less memory usage as well as faster compute times.
- Ability to easily control how fast flow increases / decreases as well as maximum possible flowrate, i.e. viscosity.

My skills at C++ coding can be generously described as amateur, this repository purpose is to show general principles of how this system functions, as well as to test how it works.

How to make your own fluid system image:
- All segments must have only 1 color.
- Blue color is a pipe.
- Green color is fluid producer, production rate is Green / 4, so RGB(0, 100, 0) is 25 fluid in per tick.
- Red color is fluid consumer, consume rate is Red / 4, so RGB(100, 0, 0) is 25 fluid out per tick.
- Background should be transparent or black.

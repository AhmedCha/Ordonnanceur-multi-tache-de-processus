# Multi-Process Scheduler Simulator

This project is a terminal-based simulator for various processus scheduling algorithms. It provides a clear and interactive way to visualize how different scheduling policies handle a set of processes. The simulator dynamically loads scheduling algorithms from shared libraries, making it easily extensible.

## Features

*   **Interactive TUI:** A simple and intuitive terminal menu to select and run different scheduling algorithms.
*   **Dynamic Algorithm Loading:** Scheduling algorithms are compiled as shared libraries and loaded at runtime, allowing for easy addition of new algorithms without recompiling the main program.
*   **Gantt Chart Visualization:** Generates a colored Gantt chart in the terminal to visualize the execution timeline of the processes.
*   **Included Scheduling Algorithms:**
    *   First-In, First-Out (FIFO)
    *   Priority-Based (Preemptive)
    *   Multi-level Feedback Queue (MLFQ)
    *   Round Robin
    *   Aging

## Installation and Usage

### Prerequisites

Before compiling the code, you need to have the following installed on your Linux machine:

*   **GCC (GNU Compiler Collection)**
*   **Make**

You can install these using your distribution's package manager. For example, on Debian-based systems (like Ubuntu):

`sudo apt-get update && sudo apt-get install build-essential`

### Running the Simulator

1.  **Clone the repository:**
    `git clone https://github.com/AhmedCha/Ordonnanceur-multi-tache-de-processus.git`
    `cd Ordonnanceur-multi-tache-de-processus/src`

2.  **Compile the project:**
    `make`

3.  **Create a process file:**
    This file defines the processes to be scheduled. Each line must follow this format:
    `process-name arrival-time execution-time initial-priority`

    An example file, `processus.txt`, is provided in the `src` directory.

4.  **Run the simulator:**
    `./main <process_file>`

    **Example:**
    `./main processus.txt`

5.  **Interactive Menu:**
    After running the command, an interactive menu will appear, allowing you to choose the desired scheduling algorithm.

    *![Screenshot of the interactive menu](./screenshots/menu.png)*

## How to Add a New Scheduling Algorithm

The scheduler is designed to be easily extensible. To add a new algorithm:

1.  **Create a C file:**
    Create a new `.c` file in the `src/politiques/` directory (e.g., `my_algorithm.c`).

2.  **Implement the `ordonnancer` function:**
    Your new file must include `../processus.h` and implement the following function:
    `void ordonnancer(Processus T[], int n)`
    *   `T` is an array of `Processus` structs.
    *   `n` is the number of processes.

    This function should contain your scheduling logic. You can refer to the existing files in `src/politiques/` for examples.

3.  **Compile and Run:**
    Simply run `make` in the `src` directory. The makefile is configured to automatically find and compile any new `.c` files in the `src/politiques/` directory into the `build/politiques/` folder. The main program will then detect and load your new algorithm.

## Project Structure

*   `src/main.c`: The main entry point of the program. Handles menu display, dynamic loading of libraries, and user interaction.
*   `src/processus.h`: Header file defining the `Processus` struct and other shared data structures.
*   `src/affichage.c`: Contains the logic for generating and displaying the Gantt chart and other results.
*   `src/politiques/`: Directory containing the source code for the different scheduling algorithms. Each file is a separate, dynamically loaded module.
*   `src/makefile`: The makefile for compiling the project.
*   `build/`: Directory where the compiled executable and shared libraries are placed. (This directory is created by the makefile).

## Acknowledgments

This project was developed with the assistance of several AI code generation tools, including OpenAI's ChatGPT, Google's Gemini, and others. Their support was instrumental in the development process.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

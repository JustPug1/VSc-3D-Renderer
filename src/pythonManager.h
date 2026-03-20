#pragma once

// PY_SSIZE_T_CLEAN must be defined before including Python.h 
// It ensures that Python uses the correct memory size types for your 64-bit system.
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Custom deleter for std::unique_ptr to handle Python objects.
 * When the unique_ptr goes out of scope, Py_XDECREF safely decrements 
 * the Python reference count, preventing memory leaks.
 */
struct PyObjectDeleter {
    void operator()(PyObject* obj) const {
        Py_XDECREF(obj);
    }
};

// Alias for a safe, auto-cleaning Python object pointer
using SmartPyPtr = std::unique_ptr<PyObject, PyObjectDeleter>;  

/**
 * @brief Data structure to hold standard drone attitude telemetry.
 */
struct DroneTelemetry {
    double roll;
    double pitch;
    double yaw;
};

/**
 * @brief Manages the lifecycle and execution of an embedded Python interpreter.
 * Uses RAII (Resource Acquisition Is Initialization) to ensure safe startup and shutdown.
 */
class PythonManager { 
public:
    // Initializes the Python environment and loads the specified module (.py script)
    PythonManager(const std::string& moduleName);
    
    // Cleans up the Python environment upon destruction
    ~PythonManager();

    // Calls a Python function that takes no arguments and returns a string
    std::string callStringFunc(const std::string& funcName);
    
    // Sends a string argument to a specific Python function
    void sendCommand(const std::string& funcName, const std::string& arg);

    // Specific bridge to pull the Roll, Pitch, and Yaw dictionary from Python
    DroneTelemetry getTelemetry();

private:
    // Holds the loaded Python script module in memory
    SmartPyPtr m_pModule;
};
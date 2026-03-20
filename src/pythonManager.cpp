#include "pythonManager.h"

PythonManager::PythonManager(const std::string& moduleName) {
        std::cout << "[Manager] Initializing Python Interpreter..." << std::endl;
        Py_Initialize();

        // Setup paths
        PyRun_SimpleString("import sys; sys.path.extend(['./python_scripts', '../python_scripts'])");

        // Load the module once and keep it alive
        SmartPyPtr pName(PyUnicode_DecodeFSDefault(moduleName.c_str()));
        m_pModule.reset(PyImport_Import(pName.get()));

        if (!m_pModule) {
            PyErr_Print();
            throw std::runtime_error("Failed to load Python module: " + moduleName);
        }
    }

PythonManager::~PythonManager() {
    std::cout << "[Manager] Shutting down Python Interpreter..." << std::endl;
    m_pModule.reset(); // Clear module first
    Py_FinalizeEx();
}

// High-performance call: we don't re-import, we just look up the function
std::string PythonManager::callStringFunc(const std::string& funcName) {
    SmartPyPtr pFunc(PyObject_GetAttrString(m_pModule.get(), funcName.c_str()));

    if (pFunc && PyCallable_Check(pFunc.get())) {
        SmartPyPtr pValue(PyObject_CallObject(pFunc.get(), nullptr));
        if (pValue) {
            return PyUnicode_AsUTF8(pValue.get());
        }
    }
    PyErr_Print();
    return "ERROR";
}

// Overload for sending commands (C++ string -> Python argument)
void PythonManager::sendCommand(const std::string& funcName, const std::string& arg) {
    SmartPyPtr pFunc(PyObject_GetAttrString(m_pModule.get(), funcName.c_str()));

    if (pFunc && PyCallable_Check(pFunc.get())) {
        SmartPyPtr pArgs(PyTuple_New(1));
        PyTuple_SetItem(pArgs.get(), 0, PyUnicode_FromString(arg.c_str())); // Steals reference

        SmartPyPtr pResult(PyObject_CallObject(pFunc.get(), pArgs.get()));
    }
}

DroneTelemetry PythonManager::getTelemetry() {
    // Initialize with safe defaults (zeros)
    DroneTelemetry data = {0.0, 0.0, 0.0};

    // 1. Get the function from the module
    SmartPyPtr pFunc(PyObject_GetAttrString(m_pModule.get(), "get_drone_attitude"));

    if (pFunc && PyCallable_Check(pFunc.get())) {
        // 2. Call the function to get the dictionary
        SmartPyPtr pDict(PyObject_CallObject(pFunc.get(), nullptr));

        // 3. Ensure we actually got a dictionary object back
        if (pDict && PyDict_Check(pDict.get())) {
            
            // PyDict_GetItemString returns a BORROWED reference. 
            // We do NOT use SmartPyPtr here because we don't own these pointers.
            PyObject* pRoll  = PyDict_GetItemString(pDict.get(), "roll");
            PyObject* pPitch = PyDict_GetItemString(pDict.get(), "pitch");
            PyObject* pYaw   = PyDict_GetItemString(pDict.get(), "yaw");

            // Convert Python Floats to C++ doubles
            if (pRoll)  data.roll  = PyFloat_AsDouble(pRoll);
            if (pPitch) data.pitch = PyFloat_AsDouble(pPitch);
            if (pYaw)   data.yaw   = PyFloat_AsDouble(pYaw);

            // If any conversion failed (e.g. data wasn't a number), Python sets an error.
            // We clear it here so it doesn't interfere with the next loop.
            if (PyErr_Occurred()) PyErr_Clear();

        } else if (PyErr_Occurred()) {
            PyErr_Print();
        }
    } else {
        PyErr_Print();
    }

    return data;
}
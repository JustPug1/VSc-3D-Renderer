#include "pythonManager.h"

PythonManager::PythonManager(const std::string& moduleName) {
    std::cout << "[Manager] Initializing Python Interpreter..." << std::endl;
    
    // Start the embedded Python interpreter
    Py_Initialize();

    // Append standard paths so Python knows where to find your custom scripts.
    // Modify these paths if your folder structure changes.
    PyRun_SimpleString("import sys; sys.path.extend(['./python_scripts', '../python_scripts'])");

    // Convert the C++ string to a Python Unicode object
    SmartPyPtr pName(PyUnicode_DecodeFSDefault(moduleName.c_str()));
    
    // Import the module (the .py file) and store it in our class member
    m_pModule.reset(PyImport_Import(pName.get()));

    // Error handling if the script is missing or has a syntax error
    if (!m_pModule) {
        PyErr_Print();
        throw std::runtime_error("Failed to load Python module: " + moduleName);
    }
}

PythonManager::~PythonManager() {
    std::cout << "[Manager] Shutting down Python Interpreter..." << std::endl;
    
    // We MUST release the module before calling Py_FinalizeEx, 
    // otherwise Python will try to clean up memory that is already destroyed.
    m_pModule.reset(); 
    Py_FinalizeEx();
}

std::string PythonManager::callStringFunc(const std::string& funcName) {
    // Look up the function by name inside our loaded module
    SmartPyPtr pFunc(PyObject_GetAttrString(m_pModule.get(), funcName.c_str()));

    // Check if the function exists and is actually callable
    if (pFunc && PyCallable_Check(pFunc.get())) {
        SmartPyPtr pValue(PyObject_CallObject(pFunc.get(), nullptr));
        
        // If the function returned a value, convert it to a C++ UTF-8 string
        if (pValue) {
            return PyUnicode_AsUTF8(pValue.get());
        }
    }
    
    PyErr_Print();
    return "ERROR";
}

void PythonManager::sendCommand(const std::string& funcName, const std::string& arg) {
    SmartPyPtr pFunc(PyObject_GetAttrString(m_pModule.get(), funcName.c_str()));

    if (pFunc && PyCallable_Check(pFunc.get())) {
        // Create a tuple to hold our arguments (size 1)
        SmartPyPtr pArgs(PyTuple_New(1));
        
        // Convert C++ string to Python string and pack it into the tuple.
        // Note: PyTuple_SetItem "steals" the reference, so we don't need a SmartPyPtr for the string itself.
        PyTuple_SetItem(pArgs.get(), 0, PyUnicode_FromString(arg.c_str())); 

        // Execute the function
        SmartPyPtr pResult(PyObject_CallObject(pFunc.get(), pArgs.get()));
    }
}

DroneTelemetry PythonManager::getTelemetry() {
    // Initialize with safe defaults (zeros) in case of communication failure
    DroneTelemetry data = {0.0, 0.0, 0.0};

    // 1. Target the specific telemetry gathering function
    SmartPyPtr pFunc(PyObject_GetAttrString(m_pModule.get(), "get_drone_attitude"));

    if (pFunc && PyCallable_Check(pFunc.get())) {
        // 2. Execute it. We expect a Python Dictionary in return.
        SmartPyPtr pDict(PyObject_CallObject(pFunc.get(), nullptr));

        // 3. Ensure we actually got a dictionary object back to avoid crashes
        if (pDict && PyDict_Check(pDict.get())) {
            
            // Extract the values using string keys.
            // WARNING: PyDict_GetItemString returns a BORROWED reference. 
            // We do NOT use SmartPyPtr here because we don't own these pointers, Python does.
            PyObject* pRoll  = PyDict_GetItemString(pDict.get(), "roll");
            PyObject* pPitch = PyDict_GetItemString(pDict.get(), "pitch");
            PyObject* pYaw   = PyDict_GetItemString(pDict.get(), "yaw");

            // Convert Python Floats to C++ doubles
            if (pRoll)  data.roll  = PyFloat_AsDouble(pRoll);
            if (pPitch) data.pitch = PyFloat_AsDouble(pPitch);
            if (pYaw)   data.yaw   = PyFloat_AsDouble(pYaw);

            // If any conversion failed (e.g., data wasn't a number), Python flags an error state.
            // We clear it here so it doesn't break the next loop iteration.
            if (PyErr_Occurred()) PyErr_Clear();

        } else if (PyErr_Occurred()) {
            PyErr_Print();
        }
    } else {
        PyErr_Print();
    }

    return data;
}
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct PyObjectDeleter {
    void operator()(PyObject* obj) const {
        Py_XDECREF(obj);
    }
};
using SmartPyPtr = std::unique_ptr<PyObject, PyObjectDeleter>;  

struct DroneTelemetry {
    double roll;
    double pitch;
    double yaw;
};

class PythonManager { 
public:
    PythonManager(const std::string& moduleName);
    ~PythonManager();

    std::string callStringFunc(const std::string& funcName);
    void sendCommand(const std::string& funcName, const std::string& arg);

    DroneTelemetry getTelemetry();

private:
    SmartPyPtr m_pModule;

};
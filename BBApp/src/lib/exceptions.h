

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

enum class ReturnValue {
    Success = 0,
    NoDeviceFound = 1,
    InvalidDeviceIndex = 2,
    InvalidArgument = 3,
    TCLAPerror = 4,
    InvalidInput = 5,
    AcquisitionError = 6,
    HardwareError = 7
};

// A multi-purpose exception used in rtl_power_fftw. It carries along an error
// message and a ReturnValue enum that will be eventually converted to an integer
// and used as a return value of the program.
class RPFexception : public std::runtime_error {
public:
    explicit RPFexception(const std::string& what, ReturnValue retval_) :
        runtime_error(what), retval(retval_) {}
    ReturnValue returnValue() const { return retval; }

private:
    ReturnValue retval;
};

#endif // EXCEPTIONS_H

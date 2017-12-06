cdef extern from "Average.c":
    int averageCalc(int n)

cpdef myAverageCalc(int n):
    print("This is my Cython wrapper")
    result = averageCalc(n)
    print("Wrapper gets result of ", result)
    return result

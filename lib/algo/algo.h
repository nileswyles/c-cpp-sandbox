#ifndef WYLESLIBS_ALGO_H
#define WYLESLIBS_ALGO_H

#ifndef ALGO_SIGNED_LONG
#define ALGO_SIGNED_LONG int64_t
#endif

#ifndef ALGO_UNSIGNED_LONG
#define ALGO_UNSIGNED_LONG uint64_t
#endif

#ifndef ALGO_SIGNED_INT
#define ALGO_SIGNED_INT int32_t
#endif

#ifndef ALGO_UNSIGNED_INT
#define ALGO_UNSIGNED_INT uint32_t
#endif

#include "datastructures/datastructures.h"

namespace WylesLibs {
    namespace Distance {
        // TODO: 
        //  Assuming sqrt(a) + sqrt(b) = sqrt(a + b) lol.. which I think I already proved to be true.

        // alright, backup...

        //  [a, b, c] <> [d, e, f]
        //      euclidean distance of two points in 3d space...
        //      is sqrt((a - d)^2 + (b - e)^2 + (c - f)^2) 
        //      
        //      and this can be extended to n-dimensions...

        //      so, euclidian of arrays (1xM matrix), can produce a single value representing the length of the hypotenuse-line-(plane?), as described above...
        //      or, represent such a vector as individual components (same as manhattan?) (maybe if polar coordinates but assuming a cartesian plane, they are related through pythogorean theorem...)

        //      let's consider the former...
        //      so, how would wwe approach this for NxM matricies? 
        //      result would be a 1xM vector representing euclidian distances between each row in the matricies...

        //      okay, this makes sense....
        //          now, manhattan is the sum of absolute values of each component
        //  [a, b, c] <manhattan> [d, e, f]
        //          = abs(a - d) + abs(b - e) + abs(c - f)

        //          = and similarly this can be extended to n-dimensions...

        //      and again if NxM matrix
        //          we will provide a 1xM vector representing manhattan distances between each row in the matricies...

        //      if you need some scalar representing, accounting for the entire matrix, one might sum(1xM vector) or avg(1xM vector)...

        //      Alright, so there's the approach... we can go ahead and implement that....

        //      alternative to both of those solutions is 1xM, NxM matrices, with differences between the two (1xM, NxM) matrices... but then athat's just subtracting two matrices...
        //          and then we can say, okay how about abs of these differences, that might be a different but useful representation of that?
        //      
        //      So, normally if might implement those as different abstractions or different operations... meaning you'll see each element more than once...
        //      think about this differently? think numpy? if not too complicated? you can specify axis?
        //          like maybe instead of each row, you might want to perform operation vertically?
        //          may also want to easily select rows/shape?
        //      or, on second thought, the beauty of this is it's simplicity? and speed

        //      maybe, learn from reader_task and allow you to chain operations on data then only visit each node once...
        //          so, that's easy for element wise operations...

        //          but, it's gets more complicated for other operations like sum, avg, etc seem similar to the collect stuff in java streams API?
        //          so, maybe something where you can specify and select axis to perform operations on (with runtime checks for shape rules based on matrix maths...) as part of the collect function?

        //          this all goes out the door if GPU acceleration is not mostly a scam for things like this... chatty stuff might not benefit from GPU acceleration, so gaming grpahics only?
        //          idk, seems like something I might want to know for sure before proceeding... like, unified memory in consoles and mac's are a thing? lol,, reading not computing? knowledge is good!

        //          yeah, theres the idea of programs (kernels) that run on other hardware, so maybe that's what I should be targeting with this library? they use a different languages typically but why not keep the same paradigm?
        //          I must be missing something...

        //      yeah, so detour.... part of reason why convolution stuff is designed as such?

        //      actually, might want both... basic computation can be performed on cpu side?
        //  
        //      okay, so similarly, create abstractions for matrix operations.
        //      
        template<typename T>
        static ALGO_UNSIGNED_INT euclidean(const MatrixVector<T>& v1, const MatrixVector<T>& v2) {
            assertArraySizes(v1, v2);
            size_t size = v1.size();
            ALGO_SIGNED_INT sum = 0;
            for (size_t i = 0; i < size; i++) {
                sum += ((ALGO_SIGNED_INT)v1[i] - v2[i]) * ((ALGO_SIGNED_INT)v1[i] - v2[i]);
            }
            return sqrt(sum);
        }
        template<typename T>
        static Matrix<ALGO_UNSIGNED_INT> euclidean(const Matrix<T>& v1, const Matrix<T>& v2) {
            size_t rows = v1.rows();
            Matrix<T> m;
            for (size_t i = 0; i < rows; i++) {
                m[i] = euclidean<T>(v1[i], v2[i]);
            } 
            return m;
        }
        template<typename T>
        static void manhattan(const MatrixVector<T>& v1, const MatrixVector<T>& v2) {
            assertArraySizes(v1, v2);
            // # no C! lol, ef the pythagorean theorem.
            size_t size = v1.size();
            MatrixVector<T> sum = 0;
            for (size_t i = 0; i < size; i++) {
                sum += abs((ALGO_SIGNED_INT)v1[i] - v2[i]);
            }
            return sqrt(sum);
        }
        template<typename T>
        static Matrix<ALGO_UNSIGNED_INT> manhattan(const Matrix<T>& v1, const Matrix<T>& v2) {
            size_t rows = v1.rows();
            Matrix<ALGO_UNSIGNED_INT> m;
            for (size_t i = 0; i < rows; i++) {
                m[i] = manhattan<T>(v1[i], v2[i]);
            } 
            return m;
        }
    };
    namespace Statistics {
        // type called DataSet
        //  Matrix<>
        //  expected_value
        //  range
        //  covariance matrix which is also includes....
        //  array of variances row?
        //  array of variances column?
        // lol, so idk maybe don't encapsulate...
        template<typename T>
        class ExpectedValue {
            private:
                double e_value;
            public:
                ExpectedValue(MatrixVector<T> a) {
                    ALGO_SIGNED_INT sum = 0;
                    size_t a_size = a.size();
                    // giggity
                    for (size_t i = 0; i < a_size; i++) {
                        sum += a[i];
                    }
                    e_value = sum/a_size;
                };
                virtual ~ExpectedValue() = default;
                virtual double value() {
                    return e_value;
                };
        };
        // TODO:
        //    algo specific array class with memoized stats of data? if so, then containerization might not be the best?
        //     so, maybe not is a, but has a? lmao
        //    yeah, no, let's think about valuable abstractions...
        static ALGO_SIGNED_LONG normalize(const ALGO_SIGNED_LONG min, const ALGO_SIGNED_LONG max, const ALGO_SIGNED_LONG value) {
            double norm_per = (double)(value - min)/(double)(max + min);
#if ALGO_SIGNED_LONG == int64_t
            ALGO_SIGNED_LONG result = UINT64_MAX * norm_per; // UINT64_MAX * value from 0 - 1
#elif ALGO_SIGNED_LONG == int32_t
            ALGO_SIGNED_LONG result = UINT32_MAX * norm_per; // UINT32_MAX * value from 0 - 1
#else
            throw std::runtime_error("Please add your own implementation here.");
#endif
            return result;
        }
        class Range {
            public:
                ALGO_SIGNED_LONG min;
                ALGO_SIGNED_LONG max;
#if ALGO_SIGNED_LONG == int64_t
                Range(): min(INT64_MAX), max(INT64_MIN) {};
#elif ALGO_SIGNED_LONG == int32_t
                Range(): min(INT32_MAX), max(INT32_MIN) {};
#else
                Range(): min(), max(ALGO) {
                    throw std::runtime_error("Please add your own implementation here.");
                };
#endif
                ~Range() = default;
        };
        // TODO: const references to Array everywhere else too... at least until encaspulation in some class or something? if at all...
        //      meaning you can't update a... a = b, but you can operate on it...
        //  also, yeah, follow older c semantics and explicitly declare "loop" variables outside of loop... 
        //  really important in things like this that require the fastest of the fastest LMAO
        template<typename T>
        static Range range(const MatrixVector<T>& a) {
            size_t size = a.size();
            Range r;
            for (size_t i = 0; i < size; i++) {
                T val = a[i];
                if (val > r.max) {
                    r.max = val;
                } else if (val < r.min) {
                    r.min = val;
                }
            }
            return r;
        }
        template<typename T>
        static MatrixVector<T> normalize(const Range r, const MatrixVector<T>& a) {
            size_t size = a.size();
            MatrixVector<T> n;
            for (size_t i = 0; i < size; i++) {
                n[i] = normalize(r.min, r.max, a[i]);
            }
            return n;
        }
        // but yeah, regarding chaining/collection abstraction... some tasks are order dependent (for the lack of a better term...) so, think about how to enforce that?
        //      reader_task, currently doesn't care... I designed with constrant that order doesn't matter.
        //  defined by function sig!!!!! lol...
        // yeah and maybe update and stringyshizzzzzzzzzzz
        // but again, that is only cool for order-dependent tasks... if I don't need to loop then don't
        template<typename T>
        static double variance(const MatrixVector<T>& a, const ExpectedValue<T>& e) {
            // average of square of distance of each element from expected value
            size_t size = a.size();
            ALGO_SIGNED_LONG variance = 0;
            for (size_t i = 0; i < size; i++) {
                variance += ((a[i] - e.e_value) * (a[i] - e.e_value));
            }
            return (double)variance/size;
        }
        template<typename T>
        static Matrix<ALGO_SIGNED_LONG> covariance(const MatrixVector<T>& a, const ExpectedValue<T>& ae, const MatrixVector<T>& b, const ExpectedValue<T>& be) {
            // average of square of distance of each element from expected value

            // TODO:
            // it is similar to convolution in that you're transposing or integrating the multiplication of two functions?...
            //  but obviously in discrete domain so, sum...
            //  and so, maybe you can generate a matrix that will do convolution? if structured properly?
            //  that's an interesting thought?
            //  same thing goes for fourier?

            // From wikipedia:
            // "Any covariance matrix is symmetric and positive semi-definite and its main diagonal contains variances (i.e., the covariance of each element with itself)."
            size_t a_size = a.size();
            size_t b_size = b.size();
            Matrix<ALGO_SIGNED_LONG> covar = 0;
            for (size_t i = 0; i < b_size; i++) {
                for (size_t j = 0; j < a_size; j++) {
                    covar += ((a[j] - ae.e_value) * (b[i] - be.e_value));
                }
            }
            return covar;
        }
    };
    namespace Space {
        // dot product
        // so, variance is dot product of itself/length of self and covariance is dot product of a * b where a = [a - e], etc.

        // orthogonality - linear independence....
        //  2 non-zero vectors are orthogonal if and only if a dot b = 0;
        template<typename T>
        static ALGO_SIGNED_LONG dotProduct(const MatrixVector<T>& a, const MatrixVector<T>& b) {
            // # a dot b = |a||b|cos(theta) 
            //  TODO: look up proof of that ^
            //  applications:  
            //      law of cosines
            assertArraySizes(a, b);

            size_t a_size = a.size();
            size_t b_size = b.size();
            ALGO_SIGNED_LONG dot = 0;
            for (size_t i = 0; i < a_size; i++) {
                dot += (a[i] * b[i]);
            }
            return dot;
        }
        template<typename T>
        static MatrixVector<T> crossProduct(const MatrixVector<T>& a, const MatrixVector<T>& b) {
            // # 
            // applications:
            //  torque
            //  fields
            if (a.size() != 3 || b.size() != 3) {
                std::runtime_error("Limit to 3 dimensions.");
            }

            MatrixVector<T> cross;
            cross += (a[1] * b[2]) - (a[2] * b[1]); // i
            cross += (a[2] * b[0]) - (a[0] * b[2]); // j
            cross += (a[0] * b[1]) - (a[1] * b[0]); // k

            return cross;
        }
        // deeterminant
        //  transpose (T)
        //  inverse (-1)
        // eigenvectors/values
        // basically review math362... and discrete math and pdfs,cdfs too. linear algebras
    };
};
#endif
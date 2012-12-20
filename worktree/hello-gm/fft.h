//   fft.h - declaration of class
//   of fast Fourier transform - FFT
//
//   The code is property of LIBROW
//   You can use it on your own
//   When utilizing credit LIBROW site

#ifndef _FFT_H_
#define _FFT_H_

namespace librow {

class complex
{
protected:
	//   Internal presentation - real and imaginary parts
	float m_re;
	float m_im;

public:
	//   Imaginary unity
	static const complex i;
	static const complex j;

	//   Constructors
	complex(): m_re(0.), m_im(0.) {}
	complex(float re, float im): m_re(re), m_im(im) {}
	complex(float val): m_re(val), m_im(0.) {}

	//   Assignment
	complex& operator= (const float val)
	{
		m_re = val;
		m_im = 0.;
		return *this;
	}

	//   Basic operations - taking parts
	float re() const { return m_re; }
	float im() const { return m_im; }

	//   Conjugate number
	complex conjugate() const
	{
		return complex(m_re, -m_im);
	}

	//   Norm   
	float norm() const
	{
		return m_re * m_re + m_im * m_im;
	}

	//   Arithmetic operations
	complex operator+ (const complex& other) const
	{
		return complex(m_re + other.m_re, m_im + other.m_im);
	}

	complex operator- (const complex& other) const
	{
		return complex(m_re - other.m_re, m_im - other.m_im);
	}

	complex operator* (const complex& other) const
	{
		return complex(m_re * other.m_re - m_im * other.m_im,
			m_re * other.m_im + m_im * other.m_re);
	}

	complex operator/ (const complex& other) const
	{
		const float denominator = other.m_re * other.m_re + other.m_im * other.m_im;
		return complex((m_re * other.m_re + m_im * other.m_im) / denominator,
			(m_im * other.m_re - m_re * other.m_im) / denominator);
	}

	complex& operator+= (const complex& other)
	{
		m_re += other.m_re;
		m_im += other.m_im;
		return *this;
	}

	complex& operator-= (const complex& other)
	{
		m_re -= other.m_re;
		m_im -= other.m_im;
		return *this;
	}

	complex& operator*= (const complex& other)
	{
		const float temp = m_re;
		m_re = m_re * other.m_re - m_im * other.m_im;
		m_im = m_im * other.m_re + temp * other.m_im;
		return *this;
	}

	complex& operator/= (const complex& other)
	{
		const float denominator = other.m_re * other.m_re + other.m_im * other.m_im;
		const float temp = m_re;
		m_re = (m_re * other.m_re + m_im * other.m_im) / denominator;
		m_im = (m_im * other.m_re - temp * other.m_im) / denominator;
		return *this;
	}

	complex& operator++ ()
	{
		++m_re;
		return *this;
	}

	complex operator++ (int)
	{
		complex temp(*this);
		++m_re;
		return temp;
	}

	complex& operator-- ()
	{
		--m_re;
		return *this;
	}

	complex operator-- (int)
	{
		complex temp(*this);
		--m_re;
		return temp;
	}

	complex operator+ (const float val) const
	{
		return complex(m_re + val, m_im);
	}

	complex operator- (const float val) const
	{
		return complex(m_re - val, m_im);
	}

	complex operator* (const float val) const
	{
		return complex(m_re * val, m_im * val);
	}

	complex operator/ (const float val) const
	{
		return complex(m_re / val, m_im / val);
	}

	complex& operator+= (const float val)
	{
		m_re += val;
		return *this;
	}

	complex& operator-= (const float val)
	{
		m_re -= val;
		return *this;
	}

	complex& operator*= (const float val)
	{
		m_re *= val;
		m_im *= val;
		return *this;
	}

	complex& operator/= (const float val)
	{
		m_re /= val;
		m_im /= val;
		return *this;
	}

	friend complex operator+ (const float left, const complex& right)
	{
		return complex(left + right.m_re, right.m_im);
	}

	friend complex operator- (const float left, const complex& right)
	{
		return complex(left - right.m_re, -right.m_im);
	}

	friend complex operator* (const float left, const complex& right)
	{
		return complex(left * right.m_re, left * right.m_im);
	}

	friend complex operator/ (const float left, const complex& right)
	{
		const float denominator = right.m_re * right.m_re + right.m_im * right.m_im;
		return complex(left * right.m_re / denominator,
			-left * right.m_im / denominator);
	}

	//   Boolean operators
	bool operator== (const complex &other) const
	{
		return m_re == other.m_re && m_im == other.m_im;
	}

	bool operator!= (const complex &other) const
	{
		return m_re != other.m_re || m_im != other.m_im;
	}

	bool operator== (const float val) const
	{
		return m_re == val && m_im == 0.;
	}

	bool operator!= (const float val) const
	{
		return m_re != val || m_im != 0.;
	}

	friend bool operator== (const float left, const complex& right)
	{
		return left == right.m_re && right.m_im == 0.;
	}

	friend bool operator!= (const float left, const complex& right)
	{
		return left != right.m_re || right.m_im != 0.;
	}
};


class CFFT
{
public:
	//   FORWARD FOURIER TRANSFORM
	//     Input  - input data
	//     Output - transform result
	//     N      - length of both input data and result
	static bool Forward(const complex *const Input, complex *const Output, const unsigned int N);

	//   FORWARD FOURIER TRANSFORM, INPLACE VERSION
	//     Data - both input data and output
	//     N    - length of input data
	static bool Forward(complex *const Data, const unsigned int N);

	//   INVERSE FOURIER TRANSFORM
	//     Input  - input data
	//     Output - transform result
	//     N      - length of both input data and result
	//     Scale  - if to scale result
	static bool Inverse(const complex *const Input, complex *const Output, const unsigned int N, const bool Scale = true);

	//   INVERSE FOURIER TRANSFORM, INPLACE VERSION
	//     Data  - both input data and output
	//     N     - length of both input data and result
	//     Scale - if to scale result
	static bool Inverse(complex *const Data, const unsigned int N, const bool Scale = true);

protected:
	//   Rearrange function and its inplace version
	static void Rearrange(const complex *const Input, complex *const Output, const unsigned int N);
	static void Rearrange(complex *const Data, const unsigned int N);

	//   FFT implementation
	static void Perform(complex *const Data, const unsigned int N, const bool Inverse = false);

	//   Scaling of inverse FFT result
	static void Scale(complex *const Data, const unsigned int N);
};

} // namespace librow

#endif

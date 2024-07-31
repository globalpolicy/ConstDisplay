#pragma once

template <typename T>
class Matrix
{
private:
	int numRows, numCols;
	T* flatArray;
	bool isVerticalAxisReversed;
public:
	Matrix(int numRows, int numCols);
	Matrix(int numRows, int numCols, const T* flatArrayInput, bool isVerticalAxisReversed = false);
	~Matrix();
	T GetElement(int x, int y) const;
	void SetElement(int x, int y, T element);
	int GetNumRows() const;
	int GetNumCols() const;
	T* GetRawArray() const;
};

template<typename T>
Matrix<T>::Matrix(int numRows, int numCols) :numRows(numRows), numCols(numCols)
{
	this->flatArray = new T[numRows * numCols];
}

template<typename T>
inline Matrix<T>::Matrix(int numRows, int numCols, const T* flatArrayInput, bool isVerticalAxisReversed)
	:numRows(numRows), numCols(numCols), isVerticalAxisReversed(isVerticalAxisReversed)
{
	this->isVerticalAxisReversed = isVerticalAxisReversed;
	this->flatArray = new T[numRows * numCols];
	for (int i = 0; i < numRows * numCols; i++) {
		this->flatArray[i] = flatArrayInput[i];
	}
}

template<typename T>
Matrix<T>::~Matrix()
{
	delete[] this->flatArray;
}

template<typename T>
T Matrix<T>::GetElement(int x, int y) const
{
	if (!this->isVerticalAxisReversed)
		return this->flatArray[y * this->numCols + x];
	else
		return this->flatArray[(this->numRows - 1 - y) * this->numCols + x];
}

template<typename T>
void Matrix<T>::SetElement(int x, int y, T element)
{
	if (!this->isVerticalAxisReversed)
		this->flatArray[x * this->numCols + y] = element;
	else
		this->flatArray[(this->numRows - 1 - x) * this->numCols + y] = element;
}

template<typename T>
int Matrix<T>::GetNumRows() const
{
	return this->numRows;
}

template<typename T>
int Matrix<T>::GetNumCols() const
{
	return this->numCols;
}

template<typename T>
T* Matrix<T>::GetRawArray() const
{
	return this->flatArray;
}

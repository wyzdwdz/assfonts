// MIT License

// Copyright (c) 2020 Vinit James

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <algorithm>
#include <iterator>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <utility>


template<typename T>
class CircularBuffer {
private:
	
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
    template <bool isConst> struct BufferIterator;
	

public:
	explicit CircularBuffer(size_t size)
		:_buff{std::unique_ptr<T[]>(new value_type[size])}, _max_size{size}{}

	CircularBuffer(const CircularBuffer& other)
		:_buff{std::unique_ptr<T[]>(new value_type[other._max_size])},
		 _max_size{other._max_size},
		 _size{other._size},
		 _head{other._head},
		 _tail{other._tail}{
			 std::copy(other.data(), other.data() + _max_size, _buff.get());
		 }

	
	CircularBuffer& operator=(const CircularBuffer& other){
		if ( this != &other){
			_buff.reset(new value_type[other._max_size]);
			_max_size = other._max_size;
			_size = other._size;
			_head = other._head;
			_tail = other._tail;
			std::copy(other.data(), other.data() + _max_size, _buff.get());
		}
		return *this;
	}

	CircularBuffer(CircularBuffer&& other) noexcept
		:_buff{std::move(other._buff)},
		 _max_size{other._max_size},
		 _size{other._size},
		 _head{other._head},
		 _tail{other._tail}{

		other._buff = nullptr;
		other._max_size = 0;
		other._size = 0;
		other._head = 0;
		other._tail = 0;
	}

	
	CircularBuffer& operator=(CircularBuffer&& other) noexcept{
		if ( this != &other){
			_buff = std::move(other._buff);
			_max_size = other._max_size;
			_size = other._size;
			_head = other._head;
			_tail = other._tail;
			
			other._buff = nullptr;
			other._max_size = 0;
			other._size = 0;
			other._head = 0;
			other._tail = 0;			
		}
		return *this;
	}
	
	void push_back(const value_type& data);
	void push_back(value_type&& data) noexcept;
	void pop_front();
	reference front();
	reference back(); 
	const_reference front() const; 
	const_reference back() const;
	void clear();
	bool empty() const ;
	bool full() const ;
	size_type capacity() const ;
	size_type size() const;
	size_type buffer_size() const {return sizeof(value_type)*_max_size;};
	const_pointer data() const { return _buff.get(); }
	
	const_reference operator[](size_type index) const;
	reference operator[](size_type index);
	const_reference at(size_type index) const;
	reference at(size_type index);

	typedef BufferIterator<false> iterator;
	typedef BufferIterator<true> const_iterator;
	
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
	const_iterator cbegin() const noexcept;
	const_iterator cend() const noexcept;
	iterator rbegin() noexcept;
	const_iterator rbegin() const noexcept;
	iterator rend() noexcept;
	const_iterator rend() const noexcept;
	
		
private:
	void _increment_bufferstate();
	void _decrement_bufferstate();
	mutable std::mutex _mtx;
	std::unique_ptr<value_type[]> _buff;
	size_type _head = 0;
	size_type _tail = 0;
	size_type _size = 0;
	size_type _max_size = 0;
			
    template<bool isConst = false>
	struct  BufferIterator{
	public:
		friend class CircularBuffer<T>;
		typedef std::random_access_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef T value_type;
		typedef typename std::conditional<isConst, const value_type&, value_type&>::type reference;
		typedef typename std::conditional<isConst, const value_type*, value_type*>::type pointer;
		typedef typename std::conditional<isConst, const CircularBuffer<value_type>*,
										  CircularBuffer<value_type>*>::type cbuf_pointer;
	private:
		cbuf_pointer _ptrToBuffer;
		size_type _offset;
		size_type _index;
		bool _reverse;
		
		bool _comparable(const BufferIterator<isConst>& other) const{
			return (_ptrToBuffer == other._ptrToBuffer)&&(_reverse == other._reverse);
		}
		
	public:
		BufferIterator()
			:_ptrToBuffer{nullptr}, _offset{0}, _index{0}, _reverse{false}{}
		
		BufferIterator(const BufferIterator<false>& it)
			:_ptrToBuffer{it._ptrToBuffer},
			 _offset{it._offset},
			 _index{it._index},
			 _reverse{it._reverse}{}

		reference operator*(){
			if(_reverse)
				return (*_ptrToBuffer)[(_ptrToBuffer->size() - _index - 1)];
			return (*_ptrToBuffer)[_index];
		}

		pointer  operator->() { return &(operator*()); }

		reference operator[](size_type index){
			BufferIterator iter = *this;
			iter._index += index;
			return *iter;
		}

		BufferIterator& operator++(){
			++_index;
			return *this;
		}

		BufferIterator operator++(int){
			BufferIterator iter = *this;
			++_index;
			return iter;
		}

		BufferIterator& operator--(){
			--_index;
			return *this;
		}

		BufferIterator operator--(int){
			BufferIterator iter = *this;
			--_index;
			return iter;
		}	

		friend BufferIterator operator+(BufferIterator lhsiter, difference_type n){
			lhsiter._index += n;
			return lhsiter;
		}

		friend BufferIterator operator+(difference_type n, BufferIterator rhsiter){
			rhsiter._index += n;
			return rhsiter;
		}
		

		BufferIterator& operator+=(difference_type n){
			_index += n;
			return *this;
		}

		friend BufferIterator operator-(BufferIterator lhsiter, difference_type n){
			lhsiter._index -= n;
			return lhsiter;
		}

		friend difference_type operator-(const BufferIterator& lhsiter, const BufferIterator& rhsiter){
			
			return lhsiter._index - rhsiter._index;
		}

		BufferIterator& operator-=(difference_type n){
			_index -= n;
			return *this;
		}

		bool operator==(const BufferIterator& other) const{
			if (!_comparable(other))
				return false;
			return ((_index == other._index)&&(_offset == other._offset));
		}
		
		bool operator!=(const BufferIterator& other) const{
			if (!_comparable(other))
				return true;
			return ((_index != other._index)||(_offset != other._offset));
		}

		bool operator<(const BufferIterator& other) const {
			if (!_comparable(other))
				return false;
			return ((_index + _offset)<(other._index+other._offset));
		}

		bool operator>(const BufferIterator& other) const{
			if (!_comparable(other))
				return false;
			return ((_index + _offset)>(other._index+other._offset));
		}

		bool operator<=(const BufferIterator& other) const {
			if (!_comparable(other))
				return false;
			return ((_index + _offset)<=(other._index+other._offset));
		}

		bool operator>=(const BufferIterator& other) const {
			if (!_comparable(other))
				return false;
			return ((_index + _offset)>=(other._index+other._offset));
		}
	};
};

template<typename T>
inline 
bool CircularBuffer<T>::full() const{
	return _size == _max_size;
}

template<typename T>
inline 
bool CircularBuffer<T>::empty() const{
	return _size == 0;
}

template<typename T>
inline 
typename CircularBuffer<T>::size_type CircularBuffer<T>::capacity() const{
	return _max_size;
}

template<typename T>
inline 
void  CircularBuffer<T>::clear(){
	std::lock_guard<std::mutex> _lck(_mtx);
	_head = _tail = _size = 0;
}

template<typename T>
inline 
typename CircularBuffer<T>::size_type CircularBuffer<T>::size() const{
	std::lock_guard<std::mutex> _lck(_mtx);
	return _size;
	}

template<typename T>
inline
typename CircularBuffer<T>::reference CircularBuffer<T>::front() {
	std::lock_guard<std::mutex> _lck(_mtx);
	if(empty())
		throw std::length_error("front function called on empty buffer");
	return _buff[_tail];
}

template<typename T>
inline
typename CircularBuffer<T>::reference CircularBuffer<T>::back() {
	std::lock_guard<std::mutex> _lck(_mtx);
	if(empty())
		throw std::length_error("back function called on empty buffer");
	return _head == 0 ? _buff[_max_size - 1] : _buff[_head - 1];
}

template<typename T>
inline
typename CircularBuffer<T>::const_reference CircularBuffer<T>::front() const{
	std::lock_guard<std::mutex> _lck(_mtx);
	if(empty())
		throw std::length_error("front function called on empty buffer");
	return _buff[_tail];
}

template<typename T>
inline
typename CircularBuffer<T>::const_reference CircularBuffer<T>::back() const{
	std::lock_guard<std::mutex> _lck(_mtx);
	if(empty())
		throw std::length_error("back function called on empty buffer");
	return _head == 0 ? _buff[_max_size - 1] : _buff[_head - 1];
}

template<typename T>
inline
void CircularBuffer<T>::push_back(const T& data){
	std::lock_guard<std::mutex> _lck(_mtx);
	//if(full())
	//	_buff[_tail].~T();
	_buff[_head] = data;
	_increment_bufferstate();
}

template<typename T>
inline
void CircularBuffer<T>::push_back(T&& data) noexcept{
	std::lock_guard<std::mutex> _lck(_mtx);
	_buff[_head] = std::move(data);
	_increment_bufferstate();
}


template<typename T>
inline 
void CircularBuffer<T>::_increment_bufferstate(){
	if(full())
		_tail = (_tail + 1)%_max_size;
	else
		++_size;
	_head = (_head + 1)%_max_size;	
}

template<typename T>
inline 
void CircularBuffer<T>::pop_front(){
	std::lock_guard<std::mutex> _lck(_mtx);
	if(empty())
		throw std::length_error("pop_front called on empty buffer");
	_decrement_bufferstate();
}

template<typename T>
inline 
void CircularBuffer<T>::_decrement_bufferstate(){
	--_size;
	_tail = (_tail + 1)%_max_size;
}

template<typename T>
inline 
typename CircularBuffer<T>::reference CircularBuffer<T>::operator[](size_t index) {
	std::lock_guard<std::mutex> _lck(_mtx);
	if((index<0)||(index>=_size))
		throw std::out_of_range("Index is out of Range of buffer size");
	index += _tail;
	index %= _max_size;
	return _buff[index];
}

template<typename T>
inline 
typename CircularBuffer<T>::const_reference CircularBuffer<T>::operator[](size_t index) const {
	std::lock_guard<std::mutex> _lck(_mtx);
	if((index<0)||(index>=_size))
		throw std::out_of_range("Index is out of Range of buffer size");
	index += _tail;
	index %= _max_size;
	return _buff[index];
}

template<typename T>
inline 
typename CircularBuffer<T>::reference CircularBuffer<T>::at(size_t index) {
	std::lock_guard<std::mutex> _lck(_mtx);
	if((index<0)||(index>=_size))
		throw std::out_of_range("Index is out of Range of buffer size");
	index += _tail;
	index %= _max_size;
	return _buff[index];
}

template<typename T>
inline 
typename CircularBuffer<T>::const_reference CircularBuffer<T>::at(size_t index) const {
	std::lock_guard<std::mutex> _lck(_mtx);
	if((index<0)||(index>=_size))
		throw std::out_of_range("Index is out of Range of buffer size");
	index += _tail;
	index %= _max_size;
	return _buff[index];
}

template<typename T>
inline 
typename CircularBuffer<T>::iterator CircularBuffer<T>::begin() {
	std::lock_guard<std::mutex> _lck(_mtx);
	iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = 0;
	iter._reverse = false;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::begin() const{
	std::lock_guard<std::mutex> _lck(_mtx);
	const_iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = 0;
	iter._reverse = false;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::iterator CircularBuffer<T>::end() {
	std::lock_guard<std::mutex> _lck(_mtx);
	iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = _size;
	iter._reverse = false;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::end() const{
	std::lock_guard<std::mutex> _lck(_mtx);
	const_iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = _size;
	iter._reverse = false;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::cbegin() const noexcept{
	std::lock_guard<std::mutex> _lck(_mtx);
	const_iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = 0;
	iter._reverse = false;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::cend() const noexcept{
	std::lock_guard<std::mutex> _lck(_mtx);
	const_iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = _size;
	iter._reverse = false;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::iterator CircularBuffer<T>::rbegin() noexcept{
	std::lock_guard<std::mutex> _lck(_mtx);
	iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = 0;
	iter._reverse = true;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::rbegin() const noexcept{
	std::lock_guard<std::mutex> _lck(_mtx);
	const_iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = 0;
	iter._reverse = true;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::iterator CircularBuffer<T>::rend()  noexcept{
	std::lock_guard<std::mutex> _lck(_mtx);
	iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = _size;
	iter._reverse = true;
	return iter;
}

template<typename T>
inline 
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::rend() const noexcept{
	std::lock_guard<std::mutex> _lck(_mtx);
	const_iterator iter;
	iter._ptrToBuffer = this;
	iter._offset = _tail;
	iter._index = _size;
	iter._reverse = true;
	return iter;
}

#endif /* CIRCULAR_BUFFER_H */
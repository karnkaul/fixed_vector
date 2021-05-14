// KT header-only library
// Requirements: C++17

#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <new>
#include <type_traits>

namespace kt {
///
/// \brief vector-like container using bytearray as storage
/// Refer to std::vector for API documentation
/// Supports: insertion, deletion (pop only), random access iteration, ranged for
/// Limitations: insert(), erase()
///
template <typename T, std::size_t N>
class fixed_vector {
	static_assert(!std::is_reference_v<T>, "T must be an object type");

  public:
	using value_type = T;
	using pointer = value_type*;
	using reference = value_type&;
	using const_pointer = value_type const*;
	using const_reference = value_type const&;

	struct iterator;
	struct const_iterator;

	static constexpr std::size_t max_size() noexcept;

	fixed_vector() = default;
	explicit fixed_vector(std::size_t count, T const& t = T{}) noexcept;
	fixed_vector(std::initializer_list<T> init) noexcept;
	fixed_vector(fixed_vector&&) noexcept;
	fixed_vector(fixed_vector const&) noexcept;
	fixed_vector& operator=(fixed_vector&&) noexcept;
	fixed_vector& operator=(fixed_vector const&) noexcept;
	~fixed_vector() noexcept;

	void push_back(value_type&& t);
	void push_back(value_type const& t);
	template <typename... Args>
	reference emplace_back(Args&&... args);
	void pop_back() noexcept;
	void clear() noexcept;

	reference front() noexcept;
	const_reference front() const noexcept;
	reference back() noexcept;
	const_reference back() const noexcept;
	reference at(std::size_t index) noexcept;
	const_reference at(std::size_t index) const noexcept;
	pointer data() noexcept;
	const_pointer data() const noexcept;

	reference operator[](std::size_t index) noexcept;
	const_reference operator[](std::size_t index) const noexcept;
	iterator begin() noexcept;
	iterator end() noexcept;
	const_iterator cbegin() const noexcept;
	const_iterator cend() const noexcept;
	const_iterator begin() const noexcept;
	const_iterator end() const noexcept;

	bool empty() const noexcept;
	std::size_t size() const noexcept;
	constexpr std::size_t capacity() const noexcept;
	bool has_space() const noexcept;

  private:
	using storage_t = std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N>;

	void clone(fixed_vector&& rhs) noexcept;
	void clone(fixed_vector const& rhs) noexcept;

	storage_t m_storage;
	std::size_t m_size = 0;

	friend struct iterator;
	friend struct const_iterator;
};

// impl

template <typename T, std::size_t N>
struct fixed_vector<T, N>::iterator {
	using iterator_category = std::random_access_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;
	using storage_t = fixed_vector<T, N>::storage_t;

	iterator() = default;

	reference operator*() noexcept { return *std::launder(reinterpret_cast<pointer>(&(*storage)[index])); }
	pointer operator->() noexcept { return std::launder(reinterpret_cast<pointer>(&(*storage)[index])); }
	iterator& operator++() noexcept {
		++index;
		return *this;
	}
	iterator& operator--() noexcept {
		--index;
		return *this;
	}
	iterator operator++(int) noexcept { return iterator(*storage, index++); }
	iterator operator--(int) noexcept { return iterator(*storage, index--); }
	iterator& operator+=(difference_type i) noexcept {
		index += i;
		return *this;
	}
	iterator operator+(difference_type i) noexcept { return iterator(*storage, index + i); }
	iterator& operator-=(difference_type i) noexcept {
		index -= i;
		return *this;
	}
	iterator operator-(difference_type i) noexcept { return iterator(*storage, index - i); }
	difference_type operator+(iterator const& rhs) noexcept { return static_cast<difference_type>(index + rhs.index); }
	difference_type operator-(iterator const& rhs) noexcept { return static_cast<difference_type>(index - rhs.index); }

	friend bool operator==(iterator lhs, iterator rhs) noexcept { return lhs.storage == rhs.storage && lhs.index == rhs.index; }
	friend bool operator!=(iterator lhs, iterator rhs) noexcept { return !(lhs == rhs); }
	friend bool operator<(iterator lhs, iterator rhs) noexcept { return lhs.index < rhs.index; }
	friend bool operator>(iterator lhs, iterator rhs) noexcept { return lhs.index > rhs.index; }
	friend bool operator<=(iterator lhs, iterator rhs) noexcept { return lhs.index <= rhs.index; }
	friend bool operator>=(iterator lhs, iterator rhs) noexcept { return lhs.index >= rhs.index; }

  private:
	iterator(storage_t& storage, std::size_t index) noexcept : storage(&storage), index(index) {}

	storage_t* storage = nullptr;
	std::size_t index = 0;

	friend class fixed_vector<T, N>;
};
template <typename T, std::size_t N>
struct fixed_vector<T, N>::const_iterator {
	using iterator_category = std::random_access_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T const*;
	using reference = T const&;
	using storage_t = fixed_vector<T, N>::storage_t;

	const_iterator() = default;

	reference operator*() const noexcept { return *std::launder(reinterpret_cast<pointer>(&(*storage)[index])); }
	pointer operator->() const noexcept { return std::launder(reinterpret_cast<pointer>(&(*storage)[index])); }
	const_iterator& operator++() noexcept {
		++index;
		return *this;
	}
	const_iterator& operator--() noexcept {
		--index;
		return *this;
	}
	const_iterator operator++(int) noexcept { return const_iterator(*storage, index++); }
	const_iterator operator--(int) noexcept { return const_iterator(*storage, index--); }
	const_iterator& operator+=(difference_type i) noexcept {
		index += i;
		return *this;
	}
	const_iterator operator+(difference_type i) noexcept { return const_iterator(*storage, index + i); }
	const_iterator& operator-=(difference_type i) noexcept {
		index -= i;
		return *this;
	}
	const_iterator operator-(difference_type i) noexcept { return const_iterator(*storage, index - i); }
	difference_type operator+(const_iterator const& rhs) noexcept { return static_cast<difference_type>(index + rhs.index); }
	difference_type operator-(const_iterator const& rhs) noexcept { return static_cast<difference_type>(index - rhs.index); }

	friend bool operator==(const_iterator lhs, const_iterator rhs) noexcept { return lhs.storage == rhs.storage && lhs.index == rhs.index; }
	friend bool operator!=(const_iterator lhs, const_iterator rhs) noexcept { return !(lhs == rhs); }
	friend bool operator<(const_iterator lhs, const_iterator rhs) noexcept { return lhs.index < rhs.index; }
	friend bool operator>(const_iterator lhs, const_iterator rhs) noexcept { return lhs.index > rhs.index; }
	friend bool operator<=(const_iterator lhs, const_iterator rhs) noexcept { return lhs.index <= rhs.index; }
	friend bool operator>=(const_iterator lhs, const_iterator rhs) noexcept { return lhs.index >= rhs.index; }

  private:
	const_iterator(storage_t const& storage, std::size_t index) noexcept : storage(&storage), index(index) {}

	storage_t const* storage = nullptr;
	std::size_t index = 0;

	friend class fixed_vector<T, N>;
};

template <typename T, std::size_t N>
fixed_vector<T, N>::fixed_vector(std::size_t count, T const& t) noexcept {
	assert(count <= capacity());
	for (std::size_t i = 0; i < count; ++i) { push_back(t); }
}
template <typename T, std::size_t N>
fixed_vector<T, N>::fixed_vector(std::initializer_list<T> init) noexcept {
	assert(init.size() <= capacity());
	for (T const& t : init) { push_back(t); }
}
template <typename T, std::size_t N>
fixed_vector<T, N>::fixed_vector(fixed_vector&& rhs) noexcept {
	clone(std::move(rhs));
	rhs.clear();
}
template <typename T, std::size_t N>
fixed_vector<T, N>::fixed_vector(fixed_vector const& rhs) noexcept {
	clone(rhs);
}
template <typename T, std::size_t N>
fixed_vector<T, N>& fixed_vector<T, N>::operator=(fixed_vector&& rhs) noexcept {
	if (&rhs != this) {
		clear();
		clone(std::move(rhs));
		rhs.clear();
	}
	return *this;
}
template <typename T, std::size_t N>
fixed_vector<T, N>& fixed_vector<T, N>::operator=(fixed_vector const& rhs) noexcept {
	if (&rhs != this) {
		clear();
		clone(rhs);
	}
	return *this;
}
template <typename T, std::size_t N>
fixed_vector<T, N>::~fixed_vector() noexcept {
	clear();
}
template <typename T, std::size_t N>
constexpr std::size_t fixed_vector<T, N>::max_size() noexcept {
	return N;
}
template <typename T, std::size_t N>
void fixed_vector<T, N>::push_back(T&& t) {
	emplace_back(std::move(t));
}
template <typename T, std::size_t N>
void fixed_vector<T, N>::push_back(T const& t) {
	emplace_back(t);
}
template <typename T, std::size_t N>
template <typename... Args>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::emplace_back(Args&&... args) {
	assert(has_space()); // size() == capacity()?
	T* t = new (&m_storage[m_size]) T(std::forward<Args>(args)...);
	++m_size;
	return *t;
}
template <typename T, std::size_t N>
void fixed_vector<T, N>::pop_back() noexcept {
	assert(!empty());
	if constexpr (!std::is_trivial_v<T>) {
		T* t = &back();
		t->~T();
	}
	--m_size;
}
template <typename T, std::size_t N>
void fixed_vector<T, N>::clear() noexcept {
	if constexpr (std::is_trivial_v<T>) {
		m_size = 0;
	} else {
		while (!empty()) { pop_back(); }
	}
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::front() noexcept {
	assert(!empty());
	return at(0);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::front() const noexcept {
	assert(!empty());
	return at(0);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::back() noexcept {
	assert(!empty());
	return at(m_size - 1);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::back() const noexcept {
	assert(!empty());
	return at(m_size - 1);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::at(std::size_t index) noexcept {
	assert(index < size());
	return *std::launder(reinterpret_cast<pointer>(&m_storage[index]));
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::at(std::size_t index) const noexcept {
	assert(index < size());
	return *std::launder(reinterpret_cast<const_pointer>(&m_storage[index]));
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::pointer fixed_vector<T, N>::data() noexcept {
	return empty() ? nullptr : &at(0);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_pointer fixed_vector<T, N>::data() const noexcept {
	return empty() ? nullptr : &at(0);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::operator[](std::size_t index) noexcept {
	return at(index);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::operator[](std::size_t index) const noexcept {
	return at(index);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::iterator fixed_vector<T, N>::begin() noexcept {
	return iterator(m_storage, 0);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::iterator fixed_vector<T, N>::end() noexcept {
	return iterator(m_storage, size());
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::begin() const noexcept {
	return const_iterator(m_storage, 0);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::end() const noexcept {
	return const_iterator(m_storage, size());
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::cbegin() const noexcept {
	return const_iterator(m_storage, 0);
}
template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::cend() const noexcept {
	return const_iterator(m_storage, size());
}
template <typename T, std::size_t N>
bool fixed_vector<T, N>::empty() const noexcept {
	return size() == 0;
}
template <typename T, std::size_t N>
std::size_t fixed_vector<T, N>::size() const noexcept {
	return m_size;
}
template <typename T, std::size_t N>
constexpr std::size_t fixed_vector<T, N>::capacity() const noexcept {
	return max_size();
}
template <typename T, std::size_t N>
bool fixed_vector<T, N>::has_space() const noexcept {
	return m_size < N;
}
template <typename T, std::size_t N>
void fixed_vector<T, N>::clone(fixed_vector&& rhs) noexcept {
	if constexpr (std::is_trivial_v<T>) {
		std::memcpy(m_storage.data(), rhs.m_storage.data(), rhs.size() * sizeof(T));
		m_size = rhs.m_size;
	} else {
		for (T& t : rhs) { push_back(std::move(t)); }
	}
}
template <typename T, std::size_t N>
void fixed_vector<T, N>::clone(fixed_vector const& rhs) noexcept {
	if constexpr (std::is_trivial_v<T>) {
		std::memcpy(m_storage.data(), rhs.m_storage.data(), rhs.size() * sizeof(T));
		m_size = rhs.m_size;
	} else {
		for (T const& t : rhs) { push_back(t); }
	}
}
} // namespace kt

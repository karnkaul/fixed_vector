// KT header-only library
// Requirements: C++17

#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <new>
#include <type_traits>

namespace kt {
///
/// \brief vector-like container using bytearray as storage
/// Refer to std::vector for API documentation
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

	constexpr fixed_vector() = default;
	constexpr explicit fixed_vector(std::size_t count, T const& t = T{}) noexcept;
	constexpr fixed_vector(std::initializer_list<T> init) noexcept;
	constexpr fixed_vector(fixed_vector&&) noexcept;
	constexpr fixed_vector(fixed_vector const&) noexcept;
	constexpr fixed_vector& operator=(fixed_vector&&) noexcept;
	constexpr fixed_vector& operator=(fixed_vector const&) noexcept;
	~fixed_vector() noexcept;

	constexpr void push_back(value_type&& t);
	constexpr void push_back(value_type const& t);
	template <typename... Args>
	constexpr reference emplace_back(Args&&... args);
	constexpr void pop_back() noexcept;
	constexpr void clear() noexcept;

	constexpr reference front() noexcept;
	constexpr const_reference front() const noexcept;
	constexpr reference back() noexcept;
	constexpr const_reference back() const noexcept;
	constexpr reference at(std::size_t index) noexcept;
	constexpr const_reference at(std::size_t index) const noexcept;
	constexpr pointer data() noexcept;
	constexpr const_pointer data() const noexcept;

	constexpr reference operator[](std::size_t index) noexcept;
	constexpr const_reference operator[](std::size_t index) const noexcept;
	constexpr iterator begin() noexcept;
	constexpr iterator end() noexcept;
	constexpr const_iterator begin() const noexcept;
	constexpr const_iterator end() const noexcept;

	constexpr bool empty() const noexcept;
	constexpr std::size_t size() const noexcept;
	constexpr std::size_t capacity() const noexcept;

  private:
	using storage_t = std::array<std::byte, sizeof(T) * N>;

	constexpr std::byte* byte_at(std::size_t idx) noexcept;
	constexpr std::byte const* byte_at(std::size_t idx) const noexcept;
	constexpr void clone(fixed_vector&& rhs) noexcept;
	constexpr void clone(fixed_vector const& rhs) noexcept;

	alignas(std::max_align_t) storage_t m_bytes;
	std::size_t m_size = 0;

	friend struct iterator;
	friend struct const_iterator;
};

// impl

template <typename T, std::size_t N>
struct fixed_vector<T, N>::iterator {
	using type = T;
	using pointer = T*;
	using reference = T&;
	using storage_t = fixed_vector<T, N>::storage_t;

	constexpr iterator() = default;

	constexpr reference operator*() noexcept {
		return *std::launder(reinterpret_cast<pointer>(&(*storage)[index]));
	}
	constexpr pointer operator->() noexcept {
		return std::launder(reinterpret_cast<pointer>(&(*storage)[index]));
	}
	constexpr iterator& operator++() noexcept {
		index += sizeof(T);
		return *this;
	}
	constexpr iterator& operator--() noexcept {
		index -= sizeof(T);
		return *this;
	}
	constexpr friend bool operator==(iterator const& lhs, iterator const& rhs) noexcept {
		return lhs.storage == rhs.storage && lhs.index == rhs.index;
	}
	constexpr friend bool operator!=(iterator const& lhs, iterator const& rhs) noexcept {
		return !(lhs == rhs);
	}

  private:
	constexpr iterator(storage_t& storage, std::size_t index) noexcept : storage(&storage), index(index) {
	}

	storage_t* storage = nullptr;
	std::size_t index = 0;

	friend class fixed_vector<T, N>;
};
template <typename T, std::size_t N>
struct fixed_vector<T, N>::const_iterator {
	using type = T;
	using pointer = T const*;
	using reference = T const&;
	using storage_t = fixed_vector<T, N>::storage_t;

	constexpr const_iterator() = default;

	constexpr reference operator*() const noexcept {
		return *std::launder(reinterpret_cast<pointer>(&(*storage)[index]));
	}
	constexpr pointer operator->() const noexcept {
		return std::launder(reinterpret_cast<pointer>(&(*storage)[index]));
	}
	constexpr const_iterator& operator++() noexcept {
		index += sizeof(T);
		return *this;
	}
	constexpr const_iterator& operator--() noexcept {
		index -= sizeof(T);
		return *this;
	}
	constexpr friend bool operator==(const_iterator const& lhs, const_iterator const& rhs) noexcept {
		return lhs.storage == rhs.storage && lhs.index == rhs.index;
	}
	constexpr friend bool operator!=(const_iterator const& lhs, const_iterator const& rhs) noexcept {
		return !(lhs == rhs);
	}

  private:
	constexpr const_iterator(storage_t const& storage, std::size_t index) noexcept : storage(&storage), index(index) {
	}

	storage_t const* storage = nullptr;
	std::size_t index = 0;

	friend class fixed_vector<T, N>;
};

template <typename T, std::size_t N>
constexpr fixed_vector<T, N>::fixed_vector(std::size_t count, T const& t) noexcept {
	assert(count <= capacity());
	for (std::size_t i = 0; i < count; ++i) {
		push_back(t);
	}
}
template <typename T, std::size_t N>
constexpr fixed_vector<T, N>::fixed_vector(std::initializer_list<T> init) noexcept {
	assert(init.size() <= capacity());
	for (T const& t : init) {
		push_back(t);
	}
}
template <typename T, std::size_t N>
constexpr fixed_vector<T, N>::fixed_vector(fixed_vector&& rhs) noexcept {
	clone(std::move(rhs));
	rhs.clear();
}
template <typename T, std::size_t N>
constexpr fixed_vector<T, N>::fixed_vector(fixed_vector const& rhs) noexcept {
	clone(rhs);
}
template <typename T, std::size_t N>
constexpr fixed_vector<T, N>& fixed_vector<T, N>::operator=(fixed_vector&& rhs) noexcept {
	if (&rhs != this) {
		clear();
		clone(std::move(rhs));
		rhs.clear();
	}
	return *this;
}
template <typename T, std::size_t N>
constexpr fixed_vector<T, N>& fixed_vector<T, N>::operator=(fixed_vector const& rhs) noexcept {
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
constexpr void fixed_vector<T, N>::push_back(T&& t) {
	emplace_back(std::move(t));
}
template <typename T, std::size_t N>
constexpr void fixed_vector<T, N>::push_back(T const& t) {
	emplace_back(t);
}
template <typename T, std::size_t N>
template <typename... Args>
constexpr typename fixed_vector<T, N>::reference fixed_vector<T, N>::emplace_back(Args&&... args) {
	assert(size() < capacity());
	std::byte* byte = byte_at(m_size++);
	T* t = new (byte) T(std::forward<Args>(args)...);
	return *t;
}
template <typename T, std::size_t N>
constexpr void fixed_vector<T, N>::pop_back() noexcept {
	assert(!empty());
	if constexpr (!std::is_trivial_v<T>) {
		T* t = &back();
		t->~T();
	}
	--m_size;
}
template <typename T, std::size_t N>
constexpr void fixed_vector<T, N>::clear() noexcept {
	if constexpr (std::is_trivial_v<T>) {
		m_size = 0;
	} else {
		while (!empty()) {
			pop_back();
		}
	}
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::reference fixed_vector<T, N>::front() noexcept {
	assert(!empty());
	return at(0);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::front() const noexcept {
	assert(!empty());
	return at(0);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::reference fixed_vector<T, N>::back() noexcept {
	assert(!empty());
	return at(m_size - 1);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::back() const noexcept {
	assert(!empty());
	return at(m_size - 1);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::reference fixed_vector<T, N>::at(std::size_t index) noexcept {
	assert(index < size());
	std::byte* byte = byte_at(index);
	return *std::launder(reinterpret_cast<pointer>(byte));
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::at(std::size_t index) const noexcept {
	assert(index < size());
	std::byte const* byte = byte_at(index);
	return *std::launder(reinterpret_cast<const_pointer>(byte));
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::pointer fixed_vector<T, N>::data() noexcept {
	return empty() ? nullptr : &at(0);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::const_pointer fixed_vector<T, N>::data() const noexcept {
	return empty() ? nullptr : &at(0);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::reference fixed_vector<T, N>::operator[](std::size_t index) noexcept {
	return at(index);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::operator[](std::size_t index) const noexcept {
	return at(index);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::iterator fixed_vector<T, N>::begin() noexcept {
	return iterator(m_bytes, 0);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::iterator fixed_vector<T, N>::end() noexcept {
	return iterator(m_bytes, size() * sizeof(T));
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::begin() const noexcept {
	return const_iterator(m_bytes, 0);
}
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::end() const noexcept {
	return const_iterator(m_bytes, size() * sizeof(T));
}
template <typename T, std::size_t N>
constexpr bool fixed_vector<T, N>::empty() const noexcept {
	return size() == 0;
}
template <typename T, std::size_t N>
constexpr std::size_t fixed_vector<T, N>::size() const noexcept {
	return m_size;
}
template <typename T, std::size_t N>
constexpr std::size_t fixed_vector<T, N>::capacity() const noexcept {
	return max_size();
}
template <typename T, std::size_t N>
constexpr std::byte* fixed_vector<T, N>::byte_at(std::size_t idx) noexcept {
	return &m_bytes[idx * sizeof(T)];
}
template <typename T, std::size_t N>
constexpr std::byte const* fixed_vector<T, N>::byte_at(std::size_t idx) const noexcept {
	return &m_bytes[idx * sizeof(T)];
}
template <typename T, std::size_t N>
constexpr void fixed_vector<T, N>::clone(fixed_vector&& rhs) noexcept {
	if constexpr (std::is_trivial_v<T>) {
		std::memcpy(m_bytes.data(), rhs.m_bytes.data(), rhs.size() * sizeof(T));
		m_size = rhs.m_size;
	} else {
		for (T& t : rhs) {
			push_back(std::move(t));
		}
	}
}
template <typename T, std::size_t N>
constexpr void fixed_vector<T, N>::clone(fixed_vector const& rhs) noexcept {
	if constexpr (std::is_trivial_v<T>) {
		std::memcpy(m_bytes.data(), rhs.m_bytes.data(), rhs.size() * sizeof(T));
		m_size = rhs.m_size;
	} else {
		for (T const& t : rhs) {
			push_back(t);
		}
	}
}
} // namespace kt

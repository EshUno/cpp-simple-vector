//шаблонный класс SimpleVector<Type>, который будет упрощённым аналогом контейнера vector
#pragma once
#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <iterator>

#include <vector>

class ReserveProxyObj {
    friend ReserveProxyObj Reserve(size_t);
    ReserveProxyObj(size_t capacity_to_reserve): capacity_(capacity_to_reserve)
    { }

    size_t capacity_ = 0;

public:
    size_t GetCapacity() const { return capacity_; }
};

template <typename Type>
class SimpleVector {
private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> ptr_;

public:
    //ДЛЯ ОТЛАДКИ
    std::vector<Type> VectorVISION(){
        std::vector<Type> vision;
        for (auto x : *this){
            vision.push_back(x);
        }
        return vision;
    }

    using Iterator = Type*;
    using ConstIterator = const Type*;

    //задает ёмкость вектора.  в векторе. Reserve сразу выделяет нужное количество памяти. При добавлении новых элементов в вектор копирование будет происходить или значительно реже или совсем не будет.
    //Если new_capacity больше текущей capacity, память должна быть перевыделена, а элементы вектора скопированы в новый отрезок памяти.
    SimpleVector(ReserveProxyObj capacity): ptr_(capacity.GetCapacity()){
        size_ = 0;
        capacity_ = capacity.GetCapacity();
    }

    void Reserve(size_t new_capacity){
       if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(ptr_.Get(), ptr_.Get() + size_, tmp.Get());
            ptr_.swap(tmp);
            capacity_ = new_capacity;

        }
    }

    //По умолчанию. Создаёт пустой вектор с нулевой вместимостью. Не выделяет динамическую память и не выбрасывает исключений.
    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size): ptr_(size) {
        size_ = size;
        capacity_ = size;
        if (ptr_) std::fill(begin(), end(), Type{});
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value): ptr_(size) {
        size_ = size;
        capacity_ = size;
        if (ptr_) std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list. Имеет размер и вместимость, совпадающую с размерами и вместимостью переданного initializer_list.
    SimpleVector(std::initializer_list<Type> init): ptr_(init.size()) {
        size_ = init.size();
        capacity_ = size_;
        std::copy(init.begin(), init.end(), ptr_.Get());
    }

    // Конструктор копирования. Копия вектора должна иметь вместимость, достаточную для хранения копии элементов исходного вектора.
    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        swap(tmp);
    }
    //Оператор присваивания. Должен обеспечивать строгую гарантию безопасности исключений.
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != & rhs){
            SimpleVector rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector&& other): ptr_(std::move(other.ptr_)){
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other. capacity_, 0);
    }

    // Перемещающий оператор присваивания
    SimpleVector& operator=(SimpleVector&& other){
        if (&other == this) return *this;

        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other. capacity_, 0);
        ptr_ = std::move(other.ptr_);

        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    // Должен обеспечивать строгую гарантию безопасности исключений.
    void PushBack(Type item) {
        size_t new_size = size_ + 1;
        size_t old_size = size_;
        if (new_size <= capacity_){
            size_ = new_size;
        }
        else{
            Resize(new_size);
        }
        ptr_[old_size] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    //  Обеспечивает базовую гарантию безопасности исключений.
    Iterator Insert(ConstIterator pos, Type value) {
        assert(pos >= begin() && pos <= end());
        auto new_pos = pos - begin();
        auto old_size = size_;
        Resize(size_ + 1);
        std::move_backward(ptr_.Get() + new_pos, ptr_.Get() + old_size, end());
        //std::copy_backward(ptr_.Get() + new_pos, ptr_.Get() + old_size, end());
        ptr_[new_pos] = std::move(value);
        return Iterator(begin() + new_pos);
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    // Не выбрасывает исключений.
    void PopBack() noexcept {
        if (!IsEmpty()) --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    // Обеспечивает базовую гарантию безопасности исключений.
    // Принимает итератор, указывающий на удаляемый элемент вектора, и возвращает итератор, который ссылается на элемент, следующий за удалённым:
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        auto new_pos = pos - begin();
        std::move(ptr_.Get() + new_pos + 1, end(), ptr_.Get() + new_pos);
        //std::copy(ptr_.Get() + new_pos + 1, end(), ptr_.Get() + new_pos);
        --size_;
        return Iterator(ptr_.Get() + new_pos);
    }

    // Обменивает значение с другим вектором
    // Не выбрасывает исключений, имеет время выполнения O(1).
    void swap(SimpleVector& other) noexcept {
        ptr_.swap(other.ptr_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // При разрушении вектора должна освобождаться память, занимаемая его элементами.
    ~SimpleVector(){

    }
    // Возвращает количество элементов в векторе. Не выбрасывает исключений.
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость вектора. Не выбрасывает исключений.
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пуст ли вектор. Не выбрасывает исключений.
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) throw std::out_of_range("Выход за пределы размеров вектора");
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) throw std::out_of_range("Выход за пределы размеров вектора");
        return ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость. Не выбрасывает исключений.
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    // Метод должен предоставлять строгую гарантию безопасности исключений.
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size < capacity_){
            //std::fill(ptr_.Get() + size_, ptr_.Get() + new_size, Type{});
            for (auto i = size_; i < new_size; ++i){
                ptr_[i] = Type{};
            }
            size_ = new_size;
            return;
        }
        auto new_cap = (capacity_ * 2 >= new_size)? (capacity_ *= 2):(capacity_ = new_size);
        ArrayPtr<Type> tmp(new_cap);
        std::move(std::make_move_iterator(begin()), std::make_move_iterator(begin() + size_), tmp.Get());
        for (auto i = size_; i < new_size; ++i){
            *(tmp.Get() + i) = Type{};
        }
        capacity_ = new_cap;
        ptr_.swap(tmp);
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    // В качестве итераторов используйте указатели
    Iterator begin() noexcept {
        return Iterator(ptr_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    // В качестве итераторов используйте указатели
    Iterator end() noexcept {
        return Iterator(ptr_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    // В качестве итераторов используйте указатели
    ConstIterator begin() const noexcept {
        return ConstIterator(ptr_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    // В качестве итераторов используйте указатели
    ConstIterator end() const noexcept {
        return ConstIterator(ptr_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    // В качестве итераторов используйте указатели
    ConstIterator cbegin() const noexcept {
        return ConstIterator(ptr_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    // В качестве итераторов используйте указатели
    ConstIterator cend() const noexcept {
        return ConstIterator(ptr_.Get() + size_);
    }
};

// Равенство вместимости не требуется.
template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (&lhs == & rhs) return true;
    if (lhs.GetSize() != rhs.GetSize()) return false;
    return (std::equal(rhs.begin(), rhs.end(), lhs.begin(), lhs.end()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}


//
// Created by xflajs00 on 15.05.2022.
//

#pragma once

#include <concepts>
#include <pf_common/Subscription.h>
#include <pf_common/specializations.h>
#include <pf_imgui/interface/Observable_impl.h>

namespace pf {

// TODO: make a special observable version for pointer like structures
// so the undeerylying value can be monitored as well, not just the pointer value

template<typename T, typename ValueType>
concept ObservableChangeDetector = std::constructible_from<T, ValueType> && requires(T t, const ValueType &newValue) {
                                                                              {
                                                                                t.hasValueChanged(newValue)
                                                                                } -> std::same_as<bool>;
                                                                            };

template<std::equality_comparable T>
class DefaultChangeDetector {
 public:
  explicit DefaultChangeDetector(const T &value) : initialValue(value) {}

  [[nodiscard]] bool hasValueChanged(const T &newValue) { return initialValue != newValue; }

 private:
  T initialValue;
};

template<direct_specialization_of<std::vector> T>
class VectorLengthChangeDetector {
 public:
  explicit VectorLengthChangeDetector(const T &value) : initialSize(value.size()) {}

  [[nodiscard]] bool hasValueChanged(const T &newValue) { return initialSize != newValue.size(); }

 private:
  typename T::size_type initialSize;
};

template<typename T, ObservableChangeDetector<T> Detector = DefaultChangeDetector<T>>
class Observable {
 public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;

  class Transaction {
   public:
    explicit Transaction(Observable &observable) : owner(observable), detector(observable.value) {
      ++owner.activeTransactions;
    }
    ~Transaction() {
      if (isLastTransaction()) {
        if (detector.hasValueChanged(owner.value)) { owner.observableImpl.notify(owner.value); }
      }
      --owner.activeTransactions;
    }

    [[nodiscard]] reference get() { return owner.value; }
    [[nodiscard]] pointer operator->() { return &owner.value; }
    [[nodiscard]] reference operator*() { return owner.value; }

   private:
    [[nodiscard]] bool isLastTransaction() const { return owner.activeTransactions == 1; }
    Observable &owner;
    Detector detector;
  };

  Observable()
    requires(std::is_default_constructible_v<value_type>)
  : value{} {}
  explicit Observable(value_type val) : value(std::move(val)) {}
  ~Observable() { destroyObservableImpl.notify(*this); }

  Observable(Observable &&other) noexcept
      : value(std::move(other.value)), observableImpl(std::move(other.observableImpl)),
        destroyObservableImpl(std::move(other.destroyObservableImpl)) {}
  Observable &operator=(Observable &&other) noexcept {
    value = std::move(other.value);
    observableImpl = std::move(other.observableImpl);
    destroyObservableImpl = std::move(other.destroyObservableImpl);
    return *this;
  }

  // explicit copy construction, because it loses observers
  [[nodiscard]] Observable copy() const { return Observable{value}; }

  Subscription addValueListener(std::invocable<const_reference> auto &&listener) {
    return observableImpl.addListener(std::forward<decltype(listener)>(listener));
  }

  Subscription addDestroyListener(std::invocable<const Observable &> auto &&listener) {
    return destroyObservableImpl.addListener(std::forward<decltype(listener)>(listener));
  }

  [[nodiscard]] const_reference operator*() const { return value; }
  [[nodiscard]] const_pointer operator->() const { return &value; }
  [[nodiscard]] const_reference get() const { return value; }

  [[nodiscard]] Transaction modify() { return Transaction{*this}; }

  [[nodiscard]] bool hasActiveTransactions() const { return activeTransactions != 0; }

 private:
  value_type value;

  std::size_t activeTransactions = 0;

  ui::ig::Observable_impl<value_type> observableImpl;
  ui::ig::Observable_impl<Observable> destroyObservableImpl;
};

template<typename Owner, typename... Args>
class ClassEvent {
  friend Owner;

 public:
  Subscription addEventListener(std::invocable<Args...> auto &&listener) {
    return observableImpl.addListener(std::forward<decltype(listener)>(listener));
  }

 private:
  void notify(const Args &...args) { observableImpl.notify(args...); }
  ui::ig::Observable_impl<Args...> observableImpl;
};

template<typename... Args>
class PublicEvent {
 public:
  Subscription addEventListener(std::invocable<Args...> auto &&listener) {
    return observableImpl.addListener(std::forward<decltype(listener)>(listener));
  }
  void notify(const Args &...args) { observableImpl.notify(args...); }

 private:
  ui::ig::Observable_impl<Args...> observableImpl;
};

}  // namespace pf

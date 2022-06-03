//
// Created by xflajs00 on 15.05.2022.
//

#pragma once

#include <concepts>
#include <pf_common/Subscription.h>
#include <pf_common/specializations.h>
#include <pf_imgui/interface/Observable_impl.h>

namespace pf {

/**
 * Object capable of detecting change of value by comparing with the old one.
 * @tparam ValueType checked type
 */
template<typename T, typename ValueType>
concept ObservableChangeDetector = std::constructible_from<T, ValueType> && requires(T t, const ValueType &newValue) {
                                                                              {
                                                                                t.hasValueChanged(newValue)
                                                                                } -> std::same_as<bool>;
                                                                            };
/**
 * Default ObservableChangeDetector. Uses operator!= to detect change.
 * @tparam T checked type
 */
template<std::equality_comparable T>
class DefaultChangeDetector {
 public:
  explicit DefaultChangeDetector(const T &value) : initialValue(value) {}

  [[nodiscard]] bool hasValueChanged(const T &newValue) { return initialValue != newValue; }

 private:
  T initialValue;
};

// FIXME: temporary concept, move to pf_common
/**
 * Check if a type has pointer like behavior.
 */
template<typename T>
concept PointerLike = std::pointer_traits<T>::element_type
    && requires(T t) {
         { *t } -> std::convertible_to<typename std::pointer_traits<T>::element_type>;
         { t.operator->() } -> std::same_as<std::add_pointer<typename std::pointer_traits<T>::element_type>>;
       };
/**
 * ObservableChangeDetector detecting pointer difference and if they're the same the inner value is compared.
 * @tparam T checked type
 */
template<PointerLike T>
  requires(
      std::equality_comparable<
          T> && std::equality_comparable_with<T, std::nullptr_t> && std::equality_comparable<typename std::pointer_traits<T>::element_type>)
class PointerAndValueChangeDetector {
 public:
  explicit PointerAndValueChangeDetector(T value) : initialValue(std::move(value)) {}

  [[nodiscard]] bool hasValueChanged(const T &newValue) {
    if (initialValue != newValue) { return true; }
    if (initialValue != nullptr && newValue != nullptr) {
      if (*initialValue != *newValue) { return true; }
    }
    return false;
  }

 private:
  T initialValue;
};

/**
 * A wrapper which allows for observing changes in inner value.
 * @tparam T stored type
 * @tparam Detector detector of value change
 */
template<typename T, ObservableChangeDetector<T> Detector = DefaultChangeDetector<T>>
class Observable {
 public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;
  /**
   * @brief Proxy for observable manipulation.
   */
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

  /**
   * Create a proxy object for manipulation of observable's value.
   */
  [[nodiscard]] Transaction modify() { return Transaction{*this}; }

  [[nodiscard]] bool hasActiveTransactions() const { return activeTransactions != 0; }

 private:
  value_type value;

  std::size_t activeTransactions = 0;

  ui::ig::Observable_impl<value_type> observableImpl;
  ui::ig::Observable_impl<Observable> destroyObservableImpl;
};

/**
 * An event which can only be triggered by its owner.
 * @tparam Owner owner
 * @tparam Args event's arguments
 */
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

/**
 * An event which can be triggered by anyone.
 * @tparam Args event's arguments
 */
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

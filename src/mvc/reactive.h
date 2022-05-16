//
// Created by xflajs00 on 15.05.2022.
//

#pragma once

#include <concepts>
#include <pf_common/Subscription.h>
#include <pf_imgui/interface/Observable_impl.h>

namespace pf {


enum class ObservableTransactionPolicy { CheckValueChange, NotifyAlways };

template<typename T, ObservableTransactionPolicy TransactionPolicy>
  requires(TransactionPolicy != ObservableTransactionPolicy::CheckValueChange
           || (std::equality_comparable<T> && std::copy_constructible<T>) )
class Observable {
  class ComparisonTransaction {
   public:
    explicit ComparisonTransaction(Observable &observable) : owner(observable), newValue(owner.value) {}
    ~ComparisonTransaction() {
      if (newValue != owner.value) {
        owner.value = newValue;
        owner.observableImpl.notify(owner.value);
      }
    }
    [[nodiscard]] T &get() { return newValue; }

   private:
    Observable &owner;
    T newValue;
  };
  class SimpleTransaction {
   public:
    explicit SimpleTransaction(Observable &observable) : owner(observable) {}
    ~SimpleTransaction() { owner.observableImpl.notify(owner.value); }
    [[nodiscard]] T &get() { return owner.value; }

   private:
    Observable &owner;
  };

 public:
  using Transaction = std::conditional_t<TransactionPolicy == ObservableTransactionPolicy::CheckValueChange,
                                         ComparisonTransaction, SimpleTransaction>;
  explicit Observable(T &&val) : value(std::forward<T>(val)) {}
  explicit Observable(T val) : value(std::move(val)) {}

  Subscription addValueListener(std::invocable<const T &> auto &&listener) {
    return observableImpl.addListener(std::forward<decltype(listener)>(listener));
  }

  [[nodiscard]] const T &getValue() const { return value; }

  [[nodiscard]] Transaction modify() { return Transaction{*this}; }

 private:
  T value;
  ui::ig::Observable_impl<T> observableImpl;
};

template<typename Owner, typename ...Args>
class Event {
  friend Owner;
 public:
  Subscription addEventListener(std::invocable<Args...> auto &&listener) {
    return observableImpl.addListener(std::forward<decltype(listener)>(listener));
  }
 private:
  void notify() {
    observableImpl.notify();
  }
  ui::ig::Observable_impl<Args...> observableImpl;
};

template<typename T>
using CheckedObservable = Observable<T, ObservableTransactionPolicy::CheckValueChange>;

}

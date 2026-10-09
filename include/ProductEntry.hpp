#include <array>
#include <stdlib.h>
#include <tuple>
#include <vector>

template <typename... Ts>
class CartesianProduct;

/**
 * @brief Class that is returned by the iteration. Gives access to the
 * elements of the vectors that make up the current combination.
 */
template <typename... Ts>
class ProductEntry {
 private:
  friend class CartesianProduct<Ts...>;
  static constexpr const std::size_t N = sizeof...(Ts);
  using Containers = std::tuple<std::vector<Ts>...>;
  using Idcs = std::array<std::size_t, N>;

  ProductEntry(const Containers& containers, const Idcs& indices)
      : m_containers(containers), m_idcs(indices) {}

 public:
  /**
   * @brief Returns element in combination
   *
   * @param IDX Index of the element
   *
   * @returns Element of vector used in the combination
   *
   * @code{.cpp}
   *   std::vector<int> x{1, 2};
   *   std::vector<int> y{10, 20};
   *   CartesianProduct prod(x, y);
   *   for (const auto& combination : prod)
   *     std::cout << combination.get<0>() << "\n"; // this will return the
   *                                                // values in `x` in the
   *                                                // current combination
   * @endcode
   *
   * @note Does bounds checking at compile time. This, of course, means that
   * we cannot supply run-time values.
   */
  template <std::size_t IDX>
  decltype(auto) get() const& {
    static_assert(IDX < N, "Dimension out of bounds.");

    return std::get<IDX>(m_containers)[m_idcs[IDX]];
  }

  /** @anchor idx
   * @brief Returns Index of element in combination
   *
   * @param IDX The dimension
   *
   * @returns Index of element of vector used in the combination
   */
  template <std::size_t IDX>
  std::size_t idx() const {
    static_assert(IDX < N, "Dimension out of bounds.");

    return m_idcs[IDX];
  }

 private:
  template <std::size_t IDX, std::size_t HELPER_IDX = N - 1>
  std::size_t productOfHigherDimensionsImpl(
      std::size_t prod = 1) const noexcept {
    static_assert((0 <= IDX) && (IDX < N));
    static_assert((0 <= HELPER_IDX) && (HELPER_IDX < N));

    prod *= std::get<HELPER_IDX>(m_containers).size();

    if constexpr (HELPER_IDX > IDX && HELPER_IDX > 0)
      return prod * productOfHigherDimensionsImpl<IDX, HELPER_IDX - 1>();
    else
      return 1;
  }

  template <std::size_t IDX = N - 1>
  std::size_t idxImpl(std::size_t sum = 0) const noexcept {
    static_assert((0 <= IDX) && (IDX < N));

    sum += productOfHigherDimensionsImpl<IDX>() * m_idcs[IDX];

    if constexpr (IDX > 0)
      return sum + idxImpl<IDX - 1>();
    else
      return sum;
  }

 public:
  /**
   * @brief Get the total index of the entry
   *
   * @returns The total index
   *
   * @details Returns the total index of the entry. This is same index as if
   * we had constructed a 1d array of all the combinations. See the code
   * example below for an example.
   *
   * @code
   *   const std::vector<int> numbers{1, 2};
   *   const std::vector<char> letters{'a', 'b'};
   *   CartesianProduct prod(numbers, letter);
   *
   *   for (const auto& e : prod) {
   *     std::cout
   *         << e.get<0>() << " " << e.get<1>()
   *         << " | indices: " << e.idx<0>() << " " << e.idx<1>()
   *         << " | total index: " << e.idx() << "\n";
   *   }
   *   // this loop will print:
   *   // 1 a | indices: 0 0 | total index: 0
   *   // 1 b | indices: 0 1 | total index: 1
   *   // 2 a | indices: 1 0 | total index: 2
   *   // 2 b | indices: 1 1 | total index: 3
   * @endcode
   *
   */
  std::size_t idx() const noexcept { return idxImpl(); }

  /**
   * @brief Non-templated version of @ref idx()
   *
   * @param IDX The dimension
   *
   * @returns Index of element of vector used in the combination
   *
   * @attention Does not do bounds checking, neither at compile time nor at
   * run time. Is also slower than @ref idx
   */
  std::size_t idx(const std::size_t idx) { return m_idcs[idx]; }

 private:
  const Containers& m_containers;
  const Idcs& m_idcs;
};

namespace std {
template <typename... Ts>
struct tuple_size<ProductEntry<Ts...>>
    : std::integral_constant<size_t, sizeof...(Ts)> {};

template <std::size_t IDX, typename... Ts>
struct tuple_element<IDX, ProductEntry<Ts...>>
    : tuple_element<IDX, tuple<Ts...>> {};
}  // namespace std

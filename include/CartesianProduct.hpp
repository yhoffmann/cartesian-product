#ifndef CARTESIAN_PRODUCT_HPP_
#define CARTESIAN_PRODUCT_HPP_

#include <array>
#include <tuple>
#include <vector>

/**
 * @brief Class that allows for iteration over cartesian-product combinations of
 * differently sized vectors of the same type. The idea is that one can use a
 * single C++ iterator to iterate over all combinations instead of writing `N`
 * different for loops
 *
 * @param T The type that the vectors hold
 * @param N The number of vectors to combine (dimension)
 */
template <typename... Ts>
class CartesianProduct {
  /**
   * @brief The number of dimesions
   */
  static constexpr const std::size_t N = sizeof...(Ts);
  using Containers = std::tuple<std::vector<Ts>...>;
  using Idcs = std::array<std::size_t, N>;

 public:
  /**
   * @brief The number of dimensions
   *
   * @returns The number of dimesion
   */
  static constexpr std::size_t numDims() noexcept { return N; }

  /**
   * @brief Get the total number of combinations in the product
   */
  std::size_t size() const noexcept { return m_size; }

  /**
   * @brief Construct from the different vectors differently
   *
   * @param vecs The vecs to build combinations from. The number of vecs to
   * supply needs to match `N`, of course
   *
   * @note The vecs are not copied. The class just saves pointers to them.
   *
   * @attention Make sure to not construct this from temporaries as this will
   * lead to undefined behaviour. TODO: Fix this at some point
   */
  explicit CartesianProduct(const std::vector<Ts>&... vecs) {
    static_assert(sizeof...(Ts) > 0, "Construct from at least one vector.");

    assign<0>(vecs...);
    calculateSize();
  }

  /**
   * @brief Class that is returned by the iteration. Gives access to the
   * elements of the vectors that make up the current combination.
   */
  class Entry {
   private:
    friend class CartesianProduct;
    Entry(const Containers& containers, const Idcs& indices)
        : m_containers(containers), m_indices(indices) {}

   public:
    /**
     * @brief Returns element in combination
     *
     * @param IDX the dimension (example below)
     *
     * @returns Element of vector used in the combination
     *
     * @code{.cpp}
     * std::vector<int> x{1, 2};
     * std::vector<int> y{10, 20};
     * CartesianProduct<int, 2> prod(x, y);
     * for (const auto& combination : prod)
     *   std::cout << combination.dim<0>() << "\n"; // this will return the
     *                                              // values in `x` in the
     *                                              // current combination
     * @endcode
     *
     * @note Does bounds checking at compile time. This, of course, means that
     * we cannot supply run-time values. Use @see T&dim(const std::size_t idx)
     * for that instead
     */
    template <std::size_t IDX>
    decltype(auto) get() const {
      static_assert(IDX < N, "Dimension out of bounds.");

      return std::get<IDX>(m_containers)[m_indices[IDX]];
    }

    /**
     * @brief Returns index of element in combination
     *
     * @param IDX the dimension (@see T& dim())
     *
     * @returns Index of element of vector used in the combination */
    template <std::size_t IDX>
    std::size_t idx() const {
      static_assert(IDX < N, "Dimension out of bounds.");

      return m_indices[IDX];
    }

    /**
     * @brief Non-templated version of @see std::site_t idx()
     *
     * @param IDX the dimension (@see T& dim())
     *
     * @returns Index of element of vector used in the combination
     *
     * @attention Does not do bounds checking, neither at compile time nor at
     * run time. Is also slower than @see std::size_t idx() */
    std::size_t idx(const std::size_t idx) { return m_indices[idx]; }

   private:
    const Containers& m_containers;
    const Idcs& m_indices;
  };

  /**
   * @brief Class the handles the iteration over combinations of entries from
   * the vectors.
   */
  class Iterator {
    template <std::size_t IDX = N - 1>
    bool iterate() {
      ++m_indices[IDX];
      if (m_indices[IDX] < std::get<IDX>(m_containers).size()) return true;
      m_indices[IDX] = 0;

      if constexpr (IDX > 0)
        return iterate<IDX - 1>();
      else
        return false;
    }

    template <std::size_t IDX = 0>
    void iterateContainers() {
      if (std::get<IDX>(m_containers).empty()) {
        m_isFinished = true;
        return;
      }

      if constexpr (IDX < N - 1) iterateContainers<IDX + 1>();
    }

   public:
    Iterator(const Containers& containers, const bool end = false)
        : m_containers(containers), m_isFinished(end) {
      m_indices.fill(0);

      if (m_isFinished) return;

      iterateContainers();
    }

    Entry operator*() const { return Entry(m_containers, m_indices); }

    Iterator& operator++() {
      if (iterate()) return *this;

      m_isFinished = true;

      return *this;
    }

    bool operator==(const Iterator& other) const {
      if (m_isFinished && other.m_isFinished) return true;

      if (m_isFinished != other.m_isFinished) return false;

      return m_indices == other.m_indices;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }

   private:
    const Containers& m_containers;
    Idcs m_indices;
    bool m_isFinished;
  };

  Iterator begin() const { return Iterator(m_containers); }

  Iterator end() const { return Iterator(m_containers, true); }

 private:
  template <std::size_t IDX, typename First, typename... Rest>
  void assign(const First& first, const Rest&... rest) {
    std::get<IDX>(m_containers) = first;

    if constexpr (sizeof...(Rest) > 0) assign<IDX + 1>(rest...);
  }

  template <std::size_t IDX = 0>
  void calculateSize() {
    if constexpr (IDX == 0) m_size = 1;

    m_size *= std::get<IDX>(m_containers).size();

    if constexpr (IDX < N - 1) calculateSize<IDX + 1>();
  }

  Containers m_containers;
  std::size_t m_size;
};  // class CartesianProduct

#endif  // CARTESIAN_PRODUCT_HPP_

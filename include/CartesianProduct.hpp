#ifndef CARTESIAN_PRODUCT_HPP_
#define CARTESIAN_PRODUCT_HPP_

#include <array>
#include <tuple>
#include <vector>

/**
 * @brief Class that allows for iteration over cartesian-product-like
 * combinations of differently sized vectors holding values of arbritrary types.
 * The idea is that one can use a single iterator to iterate over all
 * combinations instead of writing `N` different for loops.
 *
 * @param Ts The parameter pack holding the types that the individual vectors
 * hold
 *
 * @code
 *   // example usage
 *   const std::vector<int> numbers{1, 2, 3};
 *   const std::vector<std::string> strings(10, "text");
 *   const std::vector<CustomClass> customObjects(10);
 *
 *   CartesianProduct prod(numbers, strings, customObjects);
 *   prod.set<0>(std::vector<int>{-1, -2, -3});  // setting one direction
 *                                               // manually
 *
 *   for (const auto& entry : prod) {
 *     std::cout
 *         << "first component: " << entry.get<0>()
 *         << " second component: " << entry.get<1>() << " "
 *         << " index of first component: " << entry.idx<0>() << " "
 *         << " total index of this combination: " << entry.idx() << "\n";
 *   }
 *   std::cout << "total number of entries: " << prod.size() << "\n";
 *   std::cout << "number of dimensions: " << prod.numDims() << "\n";
 * @endcode
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
   * @returns The number of dimesions in O(1)
   */
  static constexpr std::size_t numDims() noexcept { return N; }

  /**
   * @brief Get the total number of combinations in the product
   *
   * @returns The total number of combinations in O(1) (calculated at
   * construction)
   */
  std::size_t size() const noexcept { return m_size; }

  /** @anchor set
   * @brief Assign a vector to a directio
   *
   * @param IDX The index of the vector to set (see example below)
   *
   * @param vec The vector to replace the current one by
   *
   * @note Can construct from temporaries as the instance create its own copy of
   * the vector.
   *
   * @attention Calling this while an Iterator currently exists will result in
   * undefined behaviour!
   *
   * @code
   *   // ... `CartesianProduct<int, double> prod` instantiated somewhere above
   *   prod.set<0>(std::vector{1, 2, 3});  // can be set via temporaries and
   *                                       // values that go out of scope etc.
   *   {
   *     const std::vector<double> x{10.0, 20.0, 30.0};
   *     prod.set<1>(x);
   *   }  // `x` goes out of scope, but `prod` owns a copy of it, still valid!
   * @endcode
   */
  template <std::size_t IDX, typename T>
  void set(const std::vector<T>& vec) {
    static_assert(
        std::is_same_v<T, std::tuple_element_t<IDX, std::tuple<Ts...>>>,
        "The input vector needs to have the same type as what the "
        "instance was constructed with.");

    std::get<IDX>(m_containers) = vec;
    calculateAndSetSize();
  }

  /**
   * @brief Default constructor
   *
   * @details Constructs with empty vectors.
   *
   * @note You will need to use @ref set to set all vectors manually.
   *
   * @note You will obviously need to supply the template arguments for the
   * class, `Ts...`, when default constructing.
   *
   * @code
   *   // supply template arguments
   *   CartesianProduct<int, double> prod;
   *
   *   // manually set each direction
   *   prod.set<0>(std::vector<int>{1, 2, 3});
   *   prod.set<1>(std::vector<double>{10.0, 20.0, 30.0});
   * @endcode
   */
  explicit CartesianProduct() {};

  /**
   * @brief Construct from the different vectors differently
   *
   * @param vecs The vecs to build combinations from. The number of vecs to
   * supply needs to match `N`, of course.
   *
   * @note The vecs are copied into the instance. No need to keep the inputs
   * alive longer than the instance.
   *
   * @code
   *   // supply the vectors to the constructor
   *   // no need to specify the types manually
   *   CartesianProduct prod(std::vector<int>{1, 2, 3},
   *       std::vector<double>{10.0, 20.0, 30.0});
   *
   *   // or from existing instances
   *   const std::vector<int> x{1, 2, 3};
   *   const std::vector<double> y{10.0, 20.0, 30.0};
   *   CartesianProduct prod2(x, y);
   * @endcode
   */
  explicit CartesianProduct(const std::vector<Ts>&... vecs) {
    static_assert(sizeof...(Ts) > 0, "Construct from at least one vector.");

    assign<0>(vecs...);
    calculateAndSetSize();
  }

  /**
   * @brief Copy constructor
   *
   * @param other Another CartesianProduct with the same template params to
   * copy.
   *
   * @note The template parameters obviously need to match between the
   * instances.
   */
  explicit CartesianProduct(const CartesianProduct<Ts...>& other)
      : m_containers(other.m_containers), m_size(other.m_size) {}

  /**
   * @brief Copy assignment
   *
   * @note The template parameters obviously need to match between the
   * instances.
   */
  CartesianProduct& operator=(const CartesianProduct<Ts...>& other) {
    if (this == &other) return *this;

    m_containers = other.m_containers;
    m_size = other.m_size;

    return *this;
  }

  /**
   * @brief Move assignment
   */
  CartesianProduct& operator=(CartesianProduct<Ts...>&& other) {
    if (this == &other) return *this;

    m_containers = std::move(other.m_containers);
    m_size = other.m_size;

    return *this;
  }

  /**
   * @brief Class that is returned by the iteration. Gives access to the
   * elements of the vectors that make up the current combination.
   */
  class Entry {
   private:
    friend class CartesianProduct;
    Entry(const Containers& containers, const Idcs& indices)
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
     *   CartesianProduct<int, 2> prod(x, y);
     *   for (const auto& combination : prod)
     *     std::cout << combination.dim<0>() << "\n"; // this will return the
     *                                                // values in `x` in the
     *                                                // current combination
     * @endcode
     *
     * @note Does bounds checking at compile time. This, of course, means that
     * we cannot supply run-time values.
     */
    template <std::size_t IDX>
    decltype(auto) get() const {
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

  /**
   * @brief Class the handles the iteration over combinations of entries from
   * the vectors.
   */
  class Iterator {
    template <std::size_t IDX = N - 1>
    bool iterate() {
      ++m_idcs[IDX];
      if (m_idcs[IDX] < std::get<IDX>(m_containers).size()) return true;
      m_idcs[IDX] = 0;

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
      m_idcs.fill(0);

      if (m_isFinished) return;

      iterateContainers();
    }

    Entry operator*() const { return Entry(m_containers, m_idcs); }

    Iterator& operator++() {
      if (iterate()) return *this;

      m_isFinished = true;

      return *this;
    }

    bool operator==(const Iterator& other) const {
      if (m_isFinished && other.m_isFinished) return true;

      if (m_isFinished != other.m_isFinished) return false;

      return m_idcs == other.m_idcs;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }

   private:
    const Containers& m_containers;
    Idcs m_idcs;
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

  void calculateAndSetSize() noexcept {
    m_size = std::apply([](const auto&... vecs) { return (vecs.size() * ...); },
                        m_containers);
  }

  Containers m_containers;
  std::size_t m_size;
};  // class CartesianProduct

#endif  // CARTESIAN_PRODUCT_HPP_

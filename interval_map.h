#pragma once

#include <map>
#include <cassert>

template<typename K, typename V>
class interval_map
{
private:
	friend void IntervalMapTest();
	V m_valBegin;
	std::map<K, V> m_map;

public:
	template<typename V_forward>
	interval_map(V_forward&& val)
		: m_valBegin(std::forward<V_forward>(val))
	{
	}

	// Assign value val to interval [keyBegin, keyEnd).
	// Overwrite previous values in this interval.
	// Conforming to the C++ Standard Library conventions, the interval
	// includes keyBegin, but excludes keyEnd.
	// If !( keyBegin < keyEnd ), this designates an empty interval,
	// and assign must do nothing.
	template<typename V_forward>
	void assign(K const& keyBegin, K const& keyEnd, V_forward&& val)
		requires (std::is_same<std::remove_cvref_t<V_forward>, V>::value)
	{
		if (!(keyBegin < keyEnd)) return;

		if (m_map.empty())
		{
			m_map.insert(std::make_pair(keyBegin, val));
			m_map.insert(std::make_pair(keyEnd, m_valBegin));
			return;
		}

		V valEnd = m_valBegin;
		const auto itEnd = m_map.upper_bound(keyEnd);
		auto itStart = itEnd;

		if (itEnd != m_map.begin())
		{
			itStart--;
			valEnd = itStart->second;
		}

		while (keyBegin < itStart->first && itStart != m_map.begin())
		{
			itStart--;
		}

		if (itStart->first < keyBegin && itStart != itEnd) itStart++;

		// check if the value differs from the preceeding range
		const bool differsFromPreviousValue = !(itStart != m_map.begin() && std::prev(itStart)->second == val);
		// check if the begin value differs from the default value
		const bool differsFromDefault = !(itStart == m_map.begin() && val == m_valBegin);

		const auto it = m_map.erase(itStart, itEnd);

		// insert begin value
		if (differsFromPreviousValue && differsFromDefault)
		{
			m_map.insert(it, std::make_pair(keyBegin, val));
		}

		// insert end value
		if (!(valEnd == val))
		{
			m_map.insert(it, std::make_pair(keyEnd, valEnd));
		}
	}

	// look-up of the value associated with key
	const V& operator[](K const& key) const
	{
		auto it = m_map.upper_bound(key);
		if (it == m_map.begin())
		{
			return m_valBegin;
		}
		else
		{
			return (--it)->second;
		}
	}

	void print() const
	{
		for (const auto& [key, val] : m_map)
		{
			std::cout << key << " : " << val << std::endl;
		}
	}
};

inline void IntervalMapTest()
{
	interval_map<int, char> imap('A');
	// Assign a single interval
	imap.assign(5, 10, 'B');
	assert(imap[4] == 'A'); // Before the range
	assert(imap[5] == 'B'); // Inside the range
	assert(imap[9] == 'B'); // End of range
	assert(imap[10] == 'A'); // After the range

	// Overlapping intervals
	imap.assign(7, 12, 'C');
	assert(imap[6] == 'B'); // Unchanged before overlap
	assert(imap[7] == 'C'); // Overwritten by new value
	assert(imap[11] == 'C'); // Inside new range
	assert(imap[12] == 'A'); // After the range

	// Contiguous intervals
	imap.assign(12, 15, 'D');
	assert(imap[12] == 'D'); // Start of new range
	assert(imap[14] == 'D'); // Inside new range
	assert(imap[15] == 'A'); // After the range

	// Redundant assignment
	imap.assign(5, 7, 'B');
	assert(imap[5] == 'B'); // Still 'B'
	assert(imap[6] == 'B'); // Still 'B'
	assert(imap[7] == 'C'); // Unchanged

	imap.assign(10, 12, 'A');

	// expected output:
	std::cout << "Expected output:\n";
	std::cout << "5 : B\n";
	std::cout << "7 : C\n";
	std::cout << "10 : A\n";
	std::cout << "12 : D\n";
	std::cout << "15 : A\n";
	std::cout << "---------------------------\n";
	std::cout << "Map values\n";
	imap.print();
	std::cout << "---------------------------\n";
	for (int i = 0; i < 17; i++)
	{
		std::cout << i << " : " << imap[i] << std::endl;
	}
}

int main()
{
	IntervalMapTest();
	return 0;
}
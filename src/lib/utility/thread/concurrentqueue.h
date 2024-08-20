/***************************************************************************
*      Mechanized Assault and Exploration Reloaded Projectfile            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef utility_concurrentqueueH
#define utility_concurrentqueueH

#include <deque>
#include <mutex>
#include <optional>

template <typename T>
class cConcurrentQueue
{
public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using size_type = std::ptrdiff_t;
	using difference_type = std::ptrdiff_t;

public:
	void push (const T& value);
	void push (T&& value);
	std::optional<T> try_pop();
	void clear();

	size_type safe_size() const;
	bool safe_empty() const;

private:
	mutable std::mutex mutex;
	std::deque<T> internalQueue;
};

//------------------------------------------------------------------------------
template <typename T>
void cConcurrentQueue<T>::push (const T& value)
{
	std::unique_lock<std::mutex> lock (mutex);

	internalQueue.push_back (value);
}

//------------------------------------------------------------------------------
template <typename T>
void cConcurrentQueue<T>::push (T&& value)
{
	std::unique_lock<std::mutex> lock (mutex);

	internalQueue.push_back (std::forward<T> (value));
}

//------------------------------------------------------------------------------
template <typename T>
std::optional<T> cConcurrentQueue<T>::try_pop()
{
	std::unique_lock<std::mutex> lock (mutex);

	if (internalQueue.empty()) return std::nullopt;

	auto res = std::move (internalQueue.front());
	internalQueue.pop_front();

	return res;
}

//------------------------------------------------------------------------------
template <typename T>
void cConcurrentQueue<T>::clear()
{
	std::unique_lock<std::mutex> lock (mutex);

	internalQueue.clear();
}

//------------------------------------------------------------------------------
template <typename T>
typename cConcurrentQueue<T>::size_type cConcurrentQueue<T>::safe_size() const
{
	std::unique_lock<std::mutex> lock (mutex);
	return internalQueue.size();
}

//------------------------------------------------------------------------------
template <typename T>
bool cConcurrentQueue<T>::safe_empty() const
{
	std::unique_lock<std::mutex> lock (mutex);
	return internalQueue.empty();
}

#endif

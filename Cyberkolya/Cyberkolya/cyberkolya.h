#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <codecvt>
#include <locale>
#include "abstract_database.h"


template <class DT = std::chrono::milliseconds,
	class ClockT = std::chrono::steady_clock>
class Timer
{
	using timep_t = typename ClockT::time_point;
	timep_t _start = ClockT::now(), _end = {};

public:
	void tick() {
		_end = timep_t{};
		_start = ClockT::now();
	}

	void tock() { _end = ClockT::now(); }

	template <class T = DT>
	auto duration() const {
		return std::chrono::duration_cast<T>(_end - _start);
	}
};


class Cyberkolya {
private:
	AbstractDatabase& db;
	Timer<> clock;
public:
	Cyberkolya(AbstractDatabase& db) : db(db) {}
	void read_from_files(const std::string& usersTsv, const std::initializer_list<std::string> postsTsvs) {
		std::wifstream file(usersTsv, std::ios::in);
		file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));
		if (!file.is_open()) {
			throw std::invalid_argument("Failed to open file");
		}
		db.load_tsv_users(file);
		file.close();
		for (const auto& tsv : postsTsvs) {
			std::wifstream file(tsv, std::ios::in);
			file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));
			if (!file.is_open()) {
				throw std::invalid_argument("Failed to open file");
			}
			db.load_tsv_posts(file);
			file.close();
		}
	}
	void process_event(std::wistream& in, std::wostream& out, std::wostream& logs = std::wcerr) {
		std::wstring type;
		in >> type;
		clock.tick();
		try {
			if (type == L"insert_user") {
				AbstractDatabase::User user{ 0, L"DELETED", L"DELETED" };
				in >> user.Id >> user.FirstName >> user.LastName;
				db.insert_user(user);
				out << L"OK" << std::endl;
			}
			else if (type == L"insert_post") {
				AbstractDatabase::Post post{ -1,-1,-1,-1, 0, 0 };
				in >> post.Id >> post.OwnerId >> post.FromId >> post.Date >> post.Likes >> post.Reposts;
				db.insert_post(post);
				out << L"OK" << std::endl;
			}
			else if (type == L"delete_post") {
				int ownerId, postId;
				in >> ownerId >> postId;
				db.delete_post(ownerId, postId);
				out << L"OK" << std::endl;
			}
			else if (type == L"like_post") {
				int ownerId, postId;
				in >> ownerId >> postId;
				db.like_post(ownerId, postId);
				out << L"OK" << std::endl;
			}
			else if (type == L"unlike_post") {
				int ownerId, postId;
				in >> ownerId >> postId;
				db.unlike_post(ownerId, postId);
				out << L"OK" << std::endl;
			}
			else if (type == L"repost_post") {
				int ownerId, postId;
				in >> ownerId >> postId;
				db.repost_post(ownerId, postId);
				out << L"OK" << std::endl;
			}
			else if (type == L"get_user") {
				int userId;
				in >> userId;
				const auto& user = db.get_user(userId);
				out << user << std::endl;
			}
			else if (type == L"get_post") {
				int ownerId, postId;
				in >> ownerId >> postId;
				const auto& post = db.get_post(ownerId, postId);
				out << post << std::endl;
			}
			else if (type == L"top_k_posts_by_likes") {
				int k, ownerId, dateBegin, dateEnd;
				in >> k >> ownerId >> dateBegin >> dateEnd;
				auto top = db.top_k_post_by_likes(k, ownerId, dateBegin, dateEnd);
				for (size_t i = 0; i < top.size(); i++) {
					out << i + 1 << ". ID: " << top[i].Id << ", likes: " << top[i].Likes << std::endl;
				}
			}
			else if (type == L"top_k_authors_by_likes") {
				int k, ownerId, dateBegin, dateEnd;
				in >> k >> ownerId >> dateBegin >> dateEnd;
				auto top = db.top_k_authors_by_likes(k, ownerId, dateBegin, dateEnd);
				for (size_t i = 0; i < top.size(); i++) {
					out << i + 1 << ". User: " << top[i].User << ", likes: " << top[i].Likes << std::endl;
				}
			}
			else if (type == L"top_k_posts_by_reposts") {
				int k, ownerId, dateBegin, dateEnd;
				in >> k >> ownerId >> dateBegin >> dateEnd;
				auto top = db.top_k_post_by_reposts(k, ownerId, dateBegin, dateEnd);
				for (size_t i = 0; i < top.size(); i++) {
					out << i + 1 << ". ID: " << top[i].Id << ", reposts: " << top[i].Reposts << std::endl;
				}
			}
			else if (type == L"top_k_authors_by_reports") {
				int k, ownerId, dateBegin, dateEnd;
				in >> k >> ownerId >> dateBegin >> dateEnd;
				auto top = db.top_k_authors_by_reports(k, ownerId, dateBegin, dateEnd);
				for (size_t i = 0; i < top.size(); i++) {
					out << i + 1 << ". User: " << top[i].User << ", reposts: " << top[i].Reposts<< std::endl;
				}
			}
			else if (type == L"help") {
				out << L"Доступные команды:" << std::endl;
				out << "get_user	userId" << std::endl;
				out << "get_post	ownerId postId" << std::endl;
				out << "insert_user	id firstName LastName" << std::endl;
				out << "insert_post	id ownerId fromId unixDate likes reposts" << std::endl;
				out << "delete_post	ownerId postId" << std::endl;
				out << "like_post	ownerId postId" << std::endl;
				out << "unlike_post	ownerId	postId" << std::endl;
				out << "repost_post	ownerId	postId" << std::endl;
				out << "top_k_post_by_likes		k	ownerId	dateBegin	dateEnd" << std::endl;
				out << "top_k_post_by_reposts		k	ownerId	dateBegin	dateEnd" << std::endl;
				out << "top_k_authors_by_likes		k	ownerId	dateBegin	dateEnd" << std::endl;
				out << "top_k_authors_by_reports	k	ownerId	dateBegin	dateEnd" << std::endl;
			}
			else {
				out << "Unknown command: " << type << std::endl;
			}
		}
		catch (const AbstractDatabase::DatabaseException& e) {
			out << "Exception: " << e.what() << std::endl;
		}
		clock.tock();
		logs << clock.duration().count() << " ms." << std::endl;
	}
};
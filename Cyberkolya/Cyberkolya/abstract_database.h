#pragma once

#include <iostream>


class AbstractDatabase {
public:
	struct Post {
		int Id;
		int OwnerId;
		int FromId;
		int Date;
		int Likes;
		int Reposts;

		bool operator==(const Post& other) const {
			return memcmp(this, &other, sizeof(Post)) == 0;
		}

		
	};

	friend std::wostream& operator<<(std::wostream& os, const AbstractDatabase::Post& post) {
		return os << post.Id << '\t' << post.Date << '\t' << post.OwnerId << '\t' << post.FromId << '\t' << post.Likes << '\t' << post.Reposts;
	}

	friend std::wistream& operator>>(std::wistream& is, AbstractDatabase::Post& post) {
		return is >> post.Id >> post.Date >> post.OwnerId >> post.FromId >> post.Likes >> post.Reposts;
	}

	struct User {
		int Id;
		std::wstring FirstName;
		std::wstring LastName;
	};

	friend std::wostream& operator<<(std::wostream& os, const AbstractDatabase::User& user) {
		return os << user.Id << '\t' << user.FirstName << '\t' << user.LastName;
	}


	friend std::wistream& operator>>(std::wistream& is, AbstractDatabase::User& user) {
		return is >> user.Id >> user.FirstName >> user.LastName;
	}

	struct UserWithLikes {
		User User;
		int Likes;
	};

	struct UserWithReposts {
		User User;
		int Reposts;
	};

	void load_tsv_posts(std::wistream& stream) {
		std::wstring headerskip; // пропускаем заголовки столбцов
		stream >> headerskip >> headerskip >> headerskip >> headerskip >> headerskip >> headerskip;
		while (!stream.eof()) {
			Post post{ -1,-1,-1,-1,-1,-1 };
			stream >> post;
			insert_post(post);
		}
	}

	void load_tsv_users(std::wistream& stream) {
		std::wstring headerskip; // пропускаем заголовки столбцов
		stream >> headerskip >> headerskip >> headerskip;
		while (!stream.eof()) {
			User user{ 0, L"DELETED", L"DELETED"};
			stream >> user;
			insert_user(user);
		}
	}

	// Объявляем свой класс исключений для базы данных
	class DatabaseException: public std::runtime_error {
	public:
		explicit DatabaseException(const std::string& message): runtime_error(message.c_str()) {}
		explicit DatabaseException(const char* message): runtime_error(message) {}
	};

	// Интерфейс базы данных
	virtual const User& get_user(int userId) = 0;
	virtual void insert_user(const User& user) = 0;
	virtual const Post& get_post(int ownerId, int postId) = 0;
	virtual void insert_post(const Post& post) = 0;
	virtual void delete_post(int ownerId, int postId) = 0;
	virtual void like_post(int ownerId, int postId) = 0;
	virtual void unlike_post(int ownerId, int postId) = 0;
	virtual void repost_post(int ownerId, int postId) = 0;
	virtual std::vector<Post> top_k_post_by_likes(int k, int ownerId, int dateBegin, int dateEnd) = 0;
	virtual std::vector<Post> top_k_post_by_reposts(int k, int ownerId, int dateBegin, int dateEnd) = 0;
	virtual std::vector<UserWithLikes> top_k_authors_by_likes(int k, int ownerId, int dateBegin, int dateEnd) = 0;
	virtual std::vector<UserWithReposts> top_k_authors_by_reports(int k, int ownerId, int dateBegin, int dateEnd) = 0;
	virtual ~AbstractDatabase() { }
};

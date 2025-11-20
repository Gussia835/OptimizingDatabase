#include "pch.h"
#include <random>
#include "CppUnitTest.h"
#include "legacy_database.h"
#include "cyberkolya.h"
#include "OptimizedDatabase.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework {
	template <>
	std::wstring ToString<AbstractDatabase::Post>(const AbstractDatabase::Post& post) {
		std::wstringstream out_stream;
		out_stream << post;
		return out_stream.str();
	}
}}}


namespace CyberkolyaTests
{
	// !! Важно !! Поменять на свой класс !!
	//using Database = LegacyDatabase;
	using Database = OptimizedDatabase;

	TEST_CLASS(Tests)
	{
	public:

		TEST_METHOD(InsertPost)
		{
			Database db;
			Database::Post post{ 1, 123, 1234, 1714676192, 0, 0 };
			db.insert_post(post);
			Assert::AreEqual(post, db.get_post(123, 1));
		}

		TEST_METHOD(MultipleInsertPost)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192, 0, 0 },
				{ 3, 123, 1514, 1712473560, 0, 0 },
				{ 2, 123, 1444, 1709579657, 0, 0 },
				{ 1, 321, 1234, 1711129565, 0, 0 },
				{ 4, 123, 1234, 1711973040, 0, 0 },
			};

			for (const auto& post: posts) db.insert_post(post);

			db.insert_post({ 2, 321, 1444, 1709129340, 0, 0 });

			for (const auto& post : posts)
				Assert::AreEqual(post, db.get_post(post.OwnerId, post.Id));
		}

		TEST_METHOD(DeletePost)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192, 0, 0 },
				{ 3, 123, 1514, 1712473560, 0, 0 },
				{ 2, 123, 1444, 1709579657, 0, 0 },
				{ 1, 321, 1234, 1711129565, 0, 0 },
				{ 4, 321, 1234, 1711973040, 0, 0 },
			};

			for (const auto& post : posts) db.insert_post(post);

			db.delete_post(321, 1);

			Assert::ExpectException<Database::DatabaseException>([&]() { db.get_post(321, 1); }, L"Неверно удаляется пост или неправильный тип исключения");
			Assert::AreEqual(posts[0], db.get_post(123, 1), L"Удалён лишний пост");
			Assert::AreEqual(posts[4], db.get_post(321, 4), L"Удалён лишний пост");
		}

		TEST_METHOD(LikePost)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192,  0, 0 },
				{ 3, 123, 1514, 1712473560,  3, 0 },
				{ 2, 123, 1444, 1709579657, 13, 0 },
				{ 1, 321, 1234, 1711129565, 10, 0 },
				{ 4, 123, 1234, 1711973040,  1, 0 },
			};

			for (const auto& post : posts) db.insert_post(post);

			for (const auto& post : posts)
				Assert::AreEqual(post.Likes, db.get_post(post.OwnerId, post.Id).Likes, L"Неправильно вставился пост с лайками");

			for (int i = 0; i < std::size(posts); ++i)
				for (int j = 0; j < i; ++j)
					db.like_post(posts[i].OwnerId, posts[i].Id);

			for (int i = 0; i < std::size(posts); ++i)
				Assert::AreEqual(posts[i].Likes + i, db.get_post(posts[i].OwnerId, posts[i].Id).Likes, L"Неправильное количество лайков после вызова like_post");
		}

		TEST_METHOD(UnlikePost)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192,  0, 0 },
				{ 3, 123, 1514, 1712473560,  3, 0 },
				{ 2, 123, 1444, 1709579657, 13, 0 },
				{ 1, 321, 1234, 1711129565, 10, 0 },
				{ 4, 123, 1234, 1711973040,  1, 0 },
			};

			for (const auto& post : posts) db.insert_post(post);

			for (const auto& post : posts)
				Assert::AreEqual(post.Likes, db.get_post(post.OwnerId, post.Id).Likes, L"Неправильно вставился пост с лайками");

			for (int i = 0; i < std::size(posts); ++i)
				for (int j = 0; j < i; ++j)
					db.unlike_post(posts[i].OwnerId, posts[i].Id);

			for (int i = 0; i < std::size(posts); ++i)
				Assert::AreEqual(posts[i].Likes - i, db.get_post(posts[i].OwnerId, posts[i].Id).Likes, L"Неправильное количество лайков после вызова unlike_post");
		}

		TEST_METHOD(RepostPost)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192, 0,  3 },
				{ 3, 123, 1514, 1712473560, 0,  0 },
				{ 2, 123, 1444, 1709579657, 0, 13 },
				{ 1, 321, 1234, 1711129565, 0, 10 },
				{ 4, 123, 1234, 1711973040, 0,  1 },
			};

			for (const auto& post : posts) db.insert_post(post);

			for (const auto& post : posts)
				Assert::AreEqual(post.Likes, db.get_post(post.OwnerId, post.Id).Likes, L"Неправильное вставился пост с репостами");

			for (int i = 0; i < std::size(posts); ++i)
				for (int j = 0; j < i; ++j)
					db.repost_post(posts[i].OwnerId, posts[i].Id);

			for (int i = 0; i < std::size(posts); ++i)
				Assert::AreEqual(posts[i].Reposts + i, db.get_post(posts[i].OwnerId, posts[i].Id).Reposts, L"Неправильное количество лайков после вызова repost_post");
		}

		TEST_METHOD(TopKPostByLikes)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192,  3, 0 },
				{ 3, 123, 1514, 1712473560,  0, 0 },
				{ 2, 123, 1444, 1709579657, 13, 0 },
				{ 5, 123, 1234, 1711129565, 10, 0 },
				{ 4, 321, 1234, 1711973040,  1, 0 },
				{ 2, 321, 1444, 1709129340,  0, 0 },
			};

			for (const auto& post : posts) db.insert_post(post);

			std::vector<Database::Post> top = db.top_k_post_by_likes(2, 123, 1711129565, 1714676193);
			Assert::AreEqual(posts[3], top[0], L"Неправильный первый пост в топе по лайкам");
			Assert::AreEqual(posts[0], top[1], L"Неправильный второй пост в топе по лайкам");
			Assert::AreEqual(2ull, top.size(), L"Неправильное количество постов в топе по лайкам");
		}

		TEST_METHOD(TopKPostByReposts)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192, 0,  3 },
				{ 3, 123, 1514, 1712473560, 0,  0 },
				{ 2, 123, 1444, 1709579657, 0, 13 },
				{ 5, 123, 1234, 1711129565, 0, 10 },
				{ 4, 321, 1234, 1711973040, 0,  1 },
				{ 2, 321, 1444, 1709129340, 0,  0 },
			};

			for (const auto& post : posts) db.insert_post(post);

			std::vector<Database::Post> top = db.top_k_post_by_reposts(3, 123, 1711129560, 1714676192);
			Assert::AreEqual(posts[3], top[0], L"Неправильный первый пост в топе по репостами");
			Assert::AreEqual(posts[1], top[1], L"Неправильный второй пост в топе по репостами");
			Assert::AreEqual(2ull, top.size(), L"Неправильное количество постов в топе по репостам");
		}

		TEST_METHOD(TopKAuthorsByLikes)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192,  3, 0 },
				{ 3, 123, 1514, 1712473560,  0, 0 },
				{ 2, 123, 1444, 1709579657, 13, 0 },
				{ 5, 123, 1234, 1711129565, 10, 0 },
				{ 4, 321, 1234, 1711973040,  1, 0 },
				{ 2, 321, 1444, 1709129340,  0, 0 },
			};

			Database::User users[] = {
				{ 1234, L"Petya", L"Ivanov"  },
				{ 1514, L"Vanya", L"Petrov"  },
				{ 1444, L"Dima",  L"Sidorow" },
			};

			for (const auto& post : posts) db.insert_post(post);
			for (const auto& user : users) db.insert_user(user);

			std::vector<Database::UserWithLikes> top = db.top_k_authors_by_likes(2, 123, 1711129565, 1714676193);
			Assert::AreEqual(13, top[0].Likes, L"Неправильное количество лайков в топе на первом месте");
			Assert::AreEqual(0, top[1].Likes, L"Неправильный количество лайков в топе на втором месте");
			Assert::AreEqual(std::wstring(L"Petya"), top[0].User.FirstName, L"Неправильный первый автор в топе по лайкам");
			Assert::AreEqual(std::wstring(L"Vanya"), top[1].User.FirstName, L"Неправильный второй автор в топе по лайкам");
			Assert::AreEqual(2ull, top.size(), L"Неправильное количество авторов в топе по лайкам");
		}

		TEST_METHOD(TopKAuthorsByLikesSameTime)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1720000000,  4, 0 },
				{ 3, 123, 1514, 1720000000,  1, 0 },
				{ 2, 123, 1444, 1720000000, 13, 0 },
				{ 5, 123, 1234, 1720000000, 10, 0 },
				{ 4, 321, 1234, 1720000000,  1, 0 },
				{ 2, 321, 1444, 1720000000, 14, 0 },
			};

			Database::User users[] = {
				{ 1234, L"Petya", L"Ivanov"  },
				{ 1514, L"Vanya", L"Petrov"  },
				{ 1444, L"Dima",  L"Sidorow" },
			};

			for (const auto& post : posts) db.insert_post(post);
			for (const auto& user : users) db.insert_user(user);

			std::vector<Database::UserWithLikes> top = db.top_k_authors_by_likes(2, 123, 1720000000, 1720000001);
			Assert::AreEqual(14, top[0].Likes, L"Неправильное количество лайков в топе на первом месте");
			Assert::AreEqual(13, top[1].Likes, L"Неправильный количество лайков в топе на втором месте");
			Assert::AreEqual(std::wstring(L"Petya"), top[0].User.FirstName, L"Неправильный первый автор в топе по лайкам");
			Assert::AreEqual(std::wstring(L"Dima"), top[1].User.FirstName, L"Неправильный второй автор в топе по лайкам");
			Assert::AreEqual(2ull, top.size(), L"Неправильное количество авторов в топе по лайкам");
		}

		TEST_METHOD(TopKAuthorsByReposts)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192, 0,  3 },
				{ 3, 123, 1514, 1712473560, 0,  0 },
				{ 2, 123, 1444, 1709579657, 0, 13 },
				{ 5, 123, 1234, 1711129565, 0, 10 },
				{ 4, 321, 1234, 1711973040, 0,  1 },
				{ 2, 321, 1444, 1709129340, 0,  0 },
			};

			Database::User users[] = {
				{ 1234, L"Petya", L"Ivanov"  },
				{ 1514, L"Vanya", L"Petrov"  },
				{ 1444, L"Dima",  L"Sidorow" },
			};

			for (const auto& post : posts) db.insert_post(post);
			for (const auto& user : users) db.insert_user(user);

			std::vector<Database::UserWithReposts> top = db.top_k_authors_by_reports(2, 123, 1711129565, 1714676193);
			Assert::AreEqual(13, top[0].Reposts, L"Неправильный количество репостов в топе на первом месте");
			Assert::AreEqual( 0, top[1].Reposts, L"Неправильный количество репостов в топе на втором месте");
			Assert::AreEqual(std::wstring(L"Vanya"), top[1].User.FirstName, L"Неправильный второй автор в топе по репостам");
			Assert::AreEqual(std::wstring(L"Petya"), top[0].User.FirstName, L"Неправильный первый автор в топе по репостам");
			Assert::AreEqual(2ull, top.size(), L"Неправильное количество авторов в топе по репостам");
		}

		TEST_METHOD(TopKAuthorsByReposts2)
		{
			Database db;
			Database::Post posts[]{
				{ 1, 123, 1234, 1714676192, 0,  3 },
				{ 3, 123, 1514, 1712473560, 0,  0 },
				{ 2, 123, 1444, 1709579657, 0, 13 },
				{ 5, 123, 1234, 1711129565, 0, 10 },
				{ 4, 321, 1234, 1711973040, 0,  1 },
				{ 2, 321, 1444, 1709129340, 0,  0 },
			};

			Database::User users[] = {
				{ 1234, L"Petya", L"Ivanov"  },
				{ 1514, L"Vanya", L"Petrov"  },
				{ 1444, L"Dima",  L"Sidorow" },
			};

			for (const auto& post : posts) db.insert_post(post);
			for (const auto& user : users) db.insert_user(user);

			std::vector<Database::UserWithReposts> top = db.top_k_authors_by_reports(2, 123, 1711129560, 1714676192);
			Assert::AreEqual(10, top[0].Reposts, L"Неправильный первый автор в топе по репостам");
			Assert::AreEqual(0, top[1].Reposts, L"Неправильный второй автор в топе по репостам");
			Assert::AreEqual(std::wstring(L"Vanya"), top[1].User.FirstName, L"Неправильный второй автор в топе по репостам");
			Assert::AreEqual(std::wstring(L"Petya"), top[0].User.FirstName, L"Неправильный первый автор в топе по репостам");
			Assert::AreEqual(2ull, top.size(), L"Неправильное количество авторов в топе по репостам");
		}
	};

	TEST_CLASS(Benchmark)
	{
		// Множитель для бенчмарков, должно работать на 1000,
		// плохая реализация базы работает только на 1
		const int k = 1000;

		// Мы написали своё распределение потому, что стандартное std::uniform_int_distribution
		// выдаёт разные результаты в зависимости от версии компилятора
		class UIntDistribution {
			std::uint32_t min, max;
		public:
			UIntDistribution(std::uint32_t min, std::uint32_t max): min(min), max(max) {}
			template <class Engine>
			std::uint32_t operator()(Engine& engine) const { return min + (engine() - Engine::min()) % (max - min); }
		};

		void load_database(AbstractDatabase& db) {
			Cyberkolya cyberkolya(db);
			cyberkolya.read_from_files("../../database/users.tsv", {
				"../../database/inside.tsv",
				"../../database/teamtardis.tsv",
				"../../database/insidenot.tsv",

				"../../database/sfedu.tsv",
				"../../database/mmcs.tsv"
			});
		}

		TEST_METHOD(InsertPost)
		{
			Database db;
			load_database(db);
			for (int i = 0; i < 1000 * k; ++i)
				db.insert_post({ i, 123, 1234, 1720000000 + i, 0, 0 });
		}

		TEST_METHOD(TopKPostByLikes)
		{
			Database db;
			load_database(db);
			std::uint64_t count = k * 1000;
			std::uint64_t hash = 0;
			for (std::uint64_t i = 0; i < count; ++i) {
				std::uint64_t shift = i * 200000000ui64 / count;
				std::vector<Database::Post> posts = db.top_k_post_by_likes(10, -39428534,
					1499000000ui64 + shift,
					1500000000ui64 + shift
				);

				for (std::uint64_t j = 0; j < posts.size(); ++j)
					hash ^= posts[j].Likes * (i * j + 1);
			}
			Assert::AreEqual(3832454783ui64, hash, L"Неправильно считается топ-k в бенчмарке");
		}

		TEST_METHOD(DeleteTopKPostByLikes)
		{
			Database db;
			load_database(db);
			const int time_start = 1400000000;
			const int time_end = 1500000000;
			const int inside_id = -39428534;

			std::mt19937 random(0);
			UIntDistribution time_dist(time_start, time_end);
			UIntDistribution like_dist(0, 10000);

			std::uint64_t count = k * 100;
			std::uint64_t hash = 0;

			for (std::uint64_t i = 0; i < count; ++i) {
				std::vector<Database::Post> posts = db.top_k_post_by_likes(10, inside_id, time_start, time_end);
				for (std::uint64_t j = 0; j < posts.size(); ++j) {
					hash ^= posts[j].Likes * (i * j + 1);
				}
				db.delete_post(posts[0].OwnerId, posts[0].Id);

				db.insert_post({ (int)i + 1000000, inside_id, 89889578, (int)time_dist(random), (int)like_dist(random), 0 });
			}
			Assert::AreEqual(122952957ui64, hash, L"Неправильно считается топ-k с удалениями");
		}

		TEST_METHOD(DeleteTopKPostByLikesReposts)
		{
			Database db;
			const int time_start = 1400000000;
			const int time_end = 1500000000;
			const int owner_id = -1;

			int db_size = 100000;
			std::uint64_t count = k * 10;
			std::uint64_t hash = 0;

			std::mt19937 random(0);
			UIntDistribution all_dist(0, 1715000000);
			UIntDistribution search_dist(time_start, time_end);
			UIntDistribution like_dist(0, 10000);
			UIntDistribution db_size_dist(0, db_size);

			std::vector<Database::Post> posts;
			for (int i = 0; i < db_size; ++i) {
				posts.push_back({ i, owner_id, 89889578, (int)all_dist(random), (int)like_dist(random), (int)like_dist(random) });
				db.insert_post(posts.back());
			}

			for (std::uint64_t i = 0; i < count; ++i) {
				std::vector<Database::Post> like_posts = db.top_k_post_by_likes(10, owner_id, time_start, time_end);
				std::vector<Database::Post> repost_posts = db.top_k_post_by_reposts(10, owner_id, time_start, time_end);
				for (std::uint64_t j = 0; j < like_posts.size(); ++j) {
					hash ^= like_posts[j].Likes * (i * j + 1);
				}
				for (std::uint64_t j = 0; j < repost_posts.size(); ++j) {
					hash ^= repost_posts[j].Reposts * (i * j + 1);
				}
				for (int j = 0; j < 100; ++j) {
					int deleted_post = db_size_dist(random);
					db.delete_post(posts[deleted_post].OwnerId, posts[deleted_post].Id);
					posts[deleted_post] = { deleted_post, owner_id, 89889578, (int)all_dist(random), (int)like_dist(random), (int)like_dist(random) };
					db.insert_post(posts[deleted_post]);
				}
			}
			Assert::AreEqual(809098601ui64, hash, L"Неправильно считается топ-k с удалениями");
		}

		TEST_METHOD(TopKAuthorsByLikes)
		{
			Database db;
			load_database(db);
			std::uint64_t count = k * 100;
			std::uint64_t hash = 0;
			for (std::uint64_t i = 0; i < count; ++i) {
				std::uint64_t shift = i * 200000000ui64 / count;
				std::vector<Database::UserWithLikes> posts = db.top_k_authors_by_likes(10, -39428534,
					1399900000ui64 + shift,
					1500000000ui64 + shift
				);

				for (std::uint64_t j = 0; j < posts.size(); ++j)
					hash ^= posts[j].Likes * (i * j + 1);
			}
			Assert::AreEqual(24929067127ui64, hash, L"Неправильно считается топ-k в бенчмарке");
		}
	};
}

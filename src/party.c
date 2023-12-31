#include <dhanda/dhanda.h>
#include <dhanda/party.h>
#include <unistd.h>


void
party_insert_in_list(dhanda *app, party *p)
{
	Node *node;

	node = list_new_node(app->party_list, p);
	if (node)
		list_insert_end(app->party_list, node);
}

party *
party_first_in_list(dhanda *app)
{
	party *p = NULL;
	Node *head;

	head = app->party_list->head;
	if (head)
		p = (party *) head->data;

	return p;
}

party *
party_second_in_list(dhanda *app)
{
	party *p = NULL;
	Node *head;

	head = app->party_list->head;
	if (head && head->next)
		p = (party *) head->next->data;

	return p;
}


int party_update_amount(struct dhanda *app, int pid, int val, int type)
{
	struct party p = {};
	int ret, sign = -1;
	char *err = NULL;
	char sql[1024];

	if (type == 0) {
		val *= sign;
	}

	sprintf(sql, "UPDATE parties SET amount = amount + %d WHERE id = %d", val, pid);

	ret = sqlite3_exec(app->db, sql, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec error: %s\n", err);
		return -1;
	}

	return 0;
}

int party_add(dhanda *app, party *party)
{
    char sql[1024];
	char *err = NULL;
	int ret;

	char *cat = created_time(party->cat);
	char *uat = updated_time(party->uat);

	sprintf(sql, "INSERT INTO parties(first_name, last_name, phone, amount, created_at, updated_at) VALUES('%s','%s', '%s', '%d', '%s', '%s')", party->fname,
		party->lname,
		party->phone,
		party->amount,
		cat,
		uat);

	ret = sqlite3_exec(app->db, sql, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec error: %s\n", err);
		//app_error_set(app, "Failed to add party");
		return -1;
	}

	party->id = sqlite3_last_insert_rowid(app->db);

	//app_success_set(app, "Party added successfully");
      return 0;
}

int party_delete(dhanda *app, party *party)
{
	int ret;
	char sql[1024];
	char *err = NULL;

	sprintf(sql, "DELETE FROM parties WHERE id = '%d'", party->id);

	ret = sqlite3_exec(app->db, sql, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec error: %s\n", err);
		return -1;
	}

	//app_success_set(app, "Party deleted successfully");
	return 0;
}

int party_findbyid(dhanda *app, int id, party *result)
{
	int ret;											/*return -1 for error*/
	int found = 0;
	char sql[1024];
	char *err = NULL;
	int is_found = 0;

	sprintf(sql, "SELECT * FROM parties WHERE id = %d", id);

	result->id = 0;
	ret = sqlite3_exec(app->db, sql, put_in_party_struct, (void *) result, &err);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "sqlite_exec: %s\n", err);
		return -1;
	}

	if (result->id == 0) {
		//app_error_set(app, "Party not found");
		return 0;
	}

	//app_success_set(app, "Party found");
	return 1;
}


int party_search(dhanda *app, char *query, struct list *result)
{
	int ret;
	char sql[1024];
	char *err = NULL;
	int is_found = 0;

	sprintf(sql, "SELECT * FROM parties WHERE phone LIKE '%%%s%%' OR first_name LIKE '%%%s%%'",
		query,
		query);

	ret = sqlite3_exec(app->db, sql, put_in_party_list, (void *) result, &err);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec: %s\n", err);
		return -1;
	}

	if (result->head == NULL) {
		//app_error_set(app, "Party not found");
		return -1;
	}

	//app_success_set(app, "Party searched successfully");

	return 0;
	
}

int put_in_party_list(void *ptr, int ncols, char **values, char **fields)
 {
 	Node *node;
 	party temp = {};

 	temp.id = atoi(values[0]);
 	strcpy(temp.fname, values[1]);
 	strcpy(temp.lname, values[2]);
 	strcpy(temp.phone, values[3]);
 	temp.amount = atoi(values[4]);
 	temp.cat = unix_time(values[5]);
 	temp.uat = unix_time(values[6]);


 	node = list_new_node((struct list *) ptr, (void *) &temp);
 	list_insert_end((struct list *) ptr, node);

 	return SQLITE_OK;
		
 }

 int put_in_party_struct(void *ptr, int ncols, char **values, char **fields)
 {
 	party *temp = (party *) ptr;
	
 	temp->id = (int) strtol(values[0], NULL, 10);
 	strcpy(temp->fname, values[1]);
 	strcpy(temp->lname, values[2]);
 	strcpy(temp->phone, values[3]);
 	temp->amount = (int) strtol(values[4], NULL, 10);
 	temp->cat = unix_time(values[5]);
 	temp->uat = unix_time(values[6]);

 	return SQLITE_OK;
		
 }
			

int party_get(dhanda *app, party_filter filter, struct list *result)
{
	int ret;
	char *err = NULL;
	char sql[1024];

	sprintf(sql, "SELECT * FROM parties");

	ret = sqlite3_exec(app->db, sql, put_in_party_list, (void *) result, &err);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec error: %s\n", err);
		return -1;
	}

	if (result->head == NULL) {
		//app_error_set(app, "Party not found");
		return 0;
	}

	//app_success_set(app, "Party found");
	return 1;
}

int party_update(dhanda *app, party *old_party, struct party *new_party)
{
	party temp;
	int matched = -1;

	int ret;
	char *err = NULL;
	char sql[1024];

	char *cat = created_time(new_party->cat);
    char *uat = updated_time(new_party->uat);

	sprintf(sql, "UPDATE parties SET "
				"first_name = '%s', last_name = '%s',"
				"phone = '%s', amount = %d,"
				"created_at = '%s', updated_at = '%s' WHERE id = %d",
														new_party->fname,
														new_party->lname,
														new_party->phone,
														new_party->amount,
														cat,
														uat,
														new_party->id);

	ret = sqlite3_exec(app->db, sql, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec error: %s\n", err);
		//app_error_set(app, "Failed to updated party");
		return -1;
	}

	//app_success_set(app, "Party updated successfully");

	return 0;
 }
 


 


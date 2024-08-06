#pragma once

//Define MARKDOWN_PARSER_IMPLEMENTATION to implement the parser inside the compilation unit

#include <string>
#include <array>

namespace markdown_parsing
{
	struct html_marks
	{
		using marks_pair = std::pair<std::string, std::string>;

		std::array<marks_pair, 6> headline_markss = {
			marks_pair{ "<h1>", "</h1>\n" },
			marks_pair{ "<h2>", "</h2>\n" },
			marks_pair{ "<h3>", "</h3>\n" },
			marks_pair{ "<h4>", "</h4>\n" },
			marks_pair{ "<h5>", "</h5>\n" },
			marks_pair{ "<h6>", "</h6>\n" },
		};

		marks_pair italic_marks = marks_pair{ "<em>", "</em>" };
		marks_pair bold_marks = marks_pair{ "<strong>", "</strong>" };
		marks_pair blockquote_marks = marks_pair{ "<blockquote>\n", "</blockquote>\n" };
		marks_pair highlight_marks = marks_pair{ "<mark>", "</mark>" };
		marks_pair strikethrough_marks = marks_pair{ "<del>", "</del>" };

		marks_pair ordered_list_marks = marks_pair{ "<ol>", "</ol>" };
		marks_pair ordered_list_item_marks = marks_pair{ "<li>", "</li>" };

		marks_pair unordered_list_marks = marks_pair{ "<ul>", "</ul>" };
		marks_pair unordered_list_item_marks = marks_pair{ "<li>", "</li>" };

		marks_pair code_marks = marks_pair{ "<code>", "</code>" };
		marks_pair code_block_marks = marks_pair{ "<pre><code>", "</code></pre>" };

		marks_pair link_additional_marks = marks_pair{ "", "" };
		marks_pair image_additional_marks = marks_pair("", "");

		std::string horizontal_rule = "<hr>";
	};

	static std::string markdown_to_html(const std::string& markdown, const html_marks& marks = {});
}

#ifdef MARKDOWN_PARSER_IMPLEMENTATION

#include <vector>
#include <sstream>

namespace markdown_parsing
{
	struct list
	{
		bool ordered;
		size_t items_indentation;
		size_t indentation_diffrence;
	};

	struct args
	{
		std::stringstream&  html_out;
		const std::string& markdown;
		const html_marks& marks;

		size_t iterator = 0;
		size_t block_begin = 0;

		size_t line_indentation = 0;

		size_t blockquote_level = 0;
		bool is_paragraph_open = 0;

		std::vector<list> ongoing_lists;
	};

	/*
		Utility
	*/

	static inline size_t count_char(char c, args& _args);
	static inline size_t get_indentation(args& _args);
	static inline bool is_ordered_list_element(args& _args);

	//Close paragraph if possible
	static inline void try_close_paragraph(args& _args);

	static inline void close_all_lists(args& _args);

	/*
		Dumping text
	*/

	//Copy text between block_begin and iterator into output
	static inline void dump_block(args& _args);

	//Dump block wrapper that adds <p> and </p>
	static inline void dump_paragraph(args& _args, bool close_paragraph);

	/*
		Parsing
	*/

	// a markdown line
	// assume iterator -> first char after \n
	static inline void parse_line(args& _args);

	// '#'
	// assumes iterator -> first '#'
	static inline void parse_headline(args& _args);

	// *a*, **a**, ***a***
	// assumes iterator -> first '*'
	static inline void parse_asteriks(args& _args);

	// ~~world is flat~~
	// assumes iterator -> char next to second '~' on the left
	static inline void parse_strikethrough(args& _args);

	// 'print("hello world")
	// assumes iterator -> '`'
	static inline void parse_code(args& _args);

	// ```py
	// abc = "hello world"
	// print(abc)
	// ```
	// assumes iterator -> char next to last '`'
	static inline void parse_code_block(args& _args);

	// ---
	// assumes iterator -> last '-'
	static inline void parse_horizontal_rule(args& _args);

	// - a
	// + a
	// * a
	// 123. a
	// assumes iterator -> '-', '*', '+' or a first element of number
	static inline void parse_list(args& _args, bool ordered);

	// <link>
	// assumes iterator -> '<'
	static inline void parse_simple_link(args& _args);

	// [title](link)
	// assumes iterator -> '['
	static inline void parse_named_link(args& _args);

	// ![alt text](file)
	// assumes iterator -> '!'
	static inline void parse_image(args& _args);
};

std::string markdown_parsing::markdown_to_html(const std::string& markdown, const html_marks& marks)
{
	std::stringstream html_out;

	args _args
	{
		.html_out = html_out,
		.markdown = markdown,
		.marks = marks
	};

	//Parse line
	while (_args.iterator < _args.markdown.size())
		parse_line(_args);				//Requires iter to be set to an char next to '\n' after previous call

	//Finish unfinished things
	try_close_paragraph(_args);
	close_all_lists(_args);

	while (_args.blockquote_level != 0)
	{
		_args.html_out << _args.marks.blockquote_marks.second;
		_args.blockquote_level--;
	}

	return html_out.str();
}

#define iter_good (_args.iterator < _args.markdown.size())

size_t markdown_parsing::count_char(char c, args& _args)
{
	size_t iterator_copy = _args.iterator;
	while (iter_good && _args.markdown.at(_args.iterator) == c) _args.iterator++;
	return _args.iterator - iterator_copy;
}

size_t markdown_parsing::get_indentation(args& _args)
{
	size_t indentation = 0;

	while (iter_good)
	{
		const auto& c = _args.markdown.at(_args.iterator);

		switch (c)
		{
		case ' ':  indentation += 1; _args.iterator++; continue;
		case '\t': indentation += 4; _args.iterator++; continue;
		}

		break;
	}

	return indentation;
}

bool markdown_parsing::is_ordered_list_element(args& _args)
{
	size_t iterator_copy = _args.iterator;
	while (iter_good && isdigit(_args.markdown.at(iterator_copy))) iterator_copy++;
	return _args.markdown.at(iterator_copy) == '.' && iterator_copy != _args.iterator;
}

void markdown_parsing::try_close_paragraph(args& _args)
{
	if (_args.is_paragraph_open)
	{
		_args.html_out << "</p>";
		_args.is_paragraph_open = false;
	}
}

void markdown_parsing::close_all_lists(args& _args)
{
	const auto& ordered_list_marks = _args.marks.ordered_list_marks;
	const auto& ordered_list_item_marks = _args.marks.ordered_list_item_marks;

	const auto& unordered_list_marks = _args.marks.unordered_list_marks;
	const auto& unordered_list_item_marks = _args.marks.unordered_list_item_marks;

	while (_args.ongoing_lists.size() != 0)
	{
		_args.html_out << (_args.ongoing_lists.back().ordered ? ordered_list_item_marks : unordered_list_item_marks).second;	//Close list item
		_args.html_out << (_args.ongoing_lists.back().ordered ? ordered_list_marks : unordered_list_marks).second;				//Close list
		_args.ongoing_lists.pop_back();
	};
}

void markdown_parsing::dump_block(args& _args)
{
	_args.html_out << _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin);
	_args.block_begin = _args.iterator;
}

void markdown_parsing::dump_paragraph(args& _args, bool close_paragraph)
{
	//If there is nothing to dump
	if (_args.iterator == _args.block_begin || (_args.iterator - _args.block_begin == 1 && _args.markdown.at(_args.iterator) == '\n'))
	{
		if (close_paragraph) try_close_paragraph(_args);
		return;
	}

	//Do not open paragraph if there is one already and if an list is open
	if (!_args.is_paragraph_open
		&& _args.ongoing_lists.size() == 0
	)
	{
		_args.html_out << "<p>";
		_args.is_paragraph_open = true;
	}

	dump_block(_args);

	if (close_paragraph && _args.is_paragraph_open)
	{
		_args.html_out << "</p>";
		_args.is_paragraph_open = false;
	}	
}

void markdown_parsing::parse_line(args& _args)
{
	//Check if line is blank
	if (_args.markdown.at(_args.iterator) == '\n')
	{
		//Close paragraph if possible
		if (_args.is_paragraph_open)
		{
			_args.html_out << "</p>";
			_args.is_paragraph_open = false;
		}
		_args.iterator++;
		return;
	}

	//Get indentation
	_args.line_indentation = get_indentation(_args);

	//Handle Blockquotes
	size_t dashes = count_char('>', _args);

	while (dashes > _args.blockquote_level)
	{
		_args.html_out << _args.marks.blockquote_marks.first;
		_args.blockquote_level++;
	}

	while (dashes < _args.blockquote_level)
	{
		_args.html_out << _args.marks.blockquote_marks.second;
		_args.blockquote_level--;
	}

	//Skip spaces
	//If there are some blockquotes get indentation again to make lists work
	if (dashes != 0)
		_args.line_indentation = get_indentation(_args);

	//If there are some opened list and this line in not a list element, then close all ongoing lists
	if  (iter_good									&& 
		_args.ongoing_lists.size() != 0				&& 
		_args.markdown.at(_args.iterator) != '-'	&& 
		_args.markdown.at(_args.iterator) != '+'	&&
		_args.markdown.at(_args.iterator) != '*'	&&
		!is_ordered_list_element(_args))	
	{
		close_all_lists(_args);
	}

	//Mark text begin
	_args.block_begin = _args.iterator;

	//Iterate
	bool is_first_char = true;
	bool ignore_next = false;

	while (iter_good && _args.markdown.at(_args.iterator) != '\n')
	{
		if (ignore_next)
		{
			ignore_next = false;
			_args.iterator++;
			continue;
		}

		const char& c = _args.markdown.at(_args.iterator);

		if (c == '\\')
		{
			dump_paragraph(_args, false);
			_args.block_begin = _args.iterator + 1;	//Move begin to the next char so the \ does not get pasted

			is_first_char = false;
			ignore_next = true;
		}
		else if (c == '#' && is_first_char)						
		{ 
			dump_paragraph(_args, true);	
			parse_headline(_args); 
		}
		else if ((c == '+' || c == '*') && is_first_char)
		{
			parse_list(_args, false);
		}
		else if (c == '*')
		{ 
			dump_paragraph(_args, false);
			parse_asteriks(_args); 
		}
		else if (c == '`')
		{ 
			size_t amount = count_char('`', _args);

			if (amount == 3)
			{
				_args.iterator -= amount;
				dump_paragraph(_args, true);
				_args.iterator += amount;
				parse_code_block(_args);
			}
			else
			{
				_args.iterator -= amount;
				dump_paragraph(_args, true);
				parse_code(_args);
			}
		}
		else if (c == '~')
		{
			size_t amount = count_char('~', _args);
			if (amount == 2)
			{
				_args.iterator -= amount;
				dump_paragraph(_args, false);
				_args.iterator += amount;
				parse_strikethrough(_args);
			}
		}
		else if (c == '-' && is_first_char)
		{
			size_t amount = count_char('-', _args);
			if (amount == 3)
			{
				_args.iterator -= amount;
				dump_paragraph(_args, true);
				_args.iterator += amount;
				parse_horizontal_rule(_args);
			}
			else
			{
				parse_list(_args, false);
			}
		}
		else if (is_first_char && is_ordered_list_element(_args))
		{
			//Skip to after '.'
			while (_args.markdown.at(_args.iterator) != '.') _args.iterator++;
			_args.iterator++;
			_args.block_begin = _args.iterator;
			parse_list(_args, true);
		}
		/*else if (c == '<')	Removed to make html injections work
		{
			dump_paragraph(_args, false);
			parse_simple_link(_args);
		}*/
		else if (c == '[')
		{
			dump_paragraph(_args, false);
			parse_named_link(_args);
		}
		else if (c == '!')
		{
			dump_paragraph(_args, false);
			parse_image(_args);
		}

		is_first_char = false;
		_args.iterator++;
	}

	dump_paragraph(_args, false);
	_args.iterator++;
}

void markdown_parsing::parse_headline(args& _args)
{
	size_t level = count_char('#', _args);

	if (_args.markdown.at(_args.iterator) != ' ') return; //No space between # and text = no headline
	_args.iterator++;									  //Skip the space though

	if (level > 6) level = 6;

	auto& marks_pair = _args.marks.headline_markss.at(level - 1);

	_args.html_out << marks_pair.first;

	_args.block_begin = _args.iterator;
	while (iter_good && _args.markdown.at(_args.iterator) != '\n') _args.iterator++;

	dump_block(_args);

	_args.html_out << marks_pair.second;
}

void markdown_parsing::parse_asteriks(args& _args)
{
	size_t left_asterisk = count_char('*', _args);

	_args.block_begin = _args.iterator;

	while (
			iter_good && 
			_args.markdown.at(_args.iterator) != '*' && 
			_args.markdown.at(_args.iterator) != '\n'
		) 
		_args.iterator++;

	if (!iter_good || _args.markdown.at(_args.iterator) == '\n')
	{
		_args.block_begin -= left_asterisk;
		return;
	}

	size_t right_asteriks = count_char('*', _args);

	while (left_asterisk > right_asteriks)
	{
		_args.html_out << '*';
		left_asterisk--;
	}

	uint8_t level = static_cast<uint8_t>(std::min(left_asterisk, right_asteriks));

	_args.iterator -= right_asteriks;
	
	switch (level)
	{
	case 1:
	{
		_args.html_out << _args.marks.italic_marks.first;
		dump_block(_args);
		_args.html_out << _args.marks.italic_marks.second;
	}
	case 2:
	{
		_args.html_out << _args.marks.bold_marks.first;
		dump_block(_args);
		_args.html_out << _args.marks.bold_marks.second;
	}
	case 3:
	{
		_args.html_out << _args.marks.italic_marks.first;
		_args.html_out << _args.marks.bold_marks.first;
		dump_block(_args);
		_args.html_out << _args.marks.bold_marks.second;
		_args.html_out << _args.marks.italic_marks.second;
	}
	}

	_args.iterator += right_asteriks - 1;
	_args.block_begin = _args.iterator;

	while (left_asterisk < right_asteriks)
	{
		_args.html_out << '*';
		right_asteriks--;
	}
}

void markdown_parsing::parse_strikethrough(args& _args)
{
	//Iterate till ~~
	_args.block_begin = _args.iterator;

	while (iter_good)
	{
		if (_args.markdown.at(_args.iterator) == '~')
		{
			size_t amount = count_char('~', _args);
			if (amount == 2) break;
		}
		_args.iterator++;
	}

	_args.html_out << _args.marks.strikethrough_marks.first;
	_args.html_out << _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin - 2);
	_args.html_out << _args.marks.strikethrough_marks.second;

	_args.block_begin = _args.iterator--;
}

void markdown_parsing::parse_code(args& _args)
{
	_args.iterator++;
	if (!iter_good) return;

	_args.block_begin = _args.iterator;

	while (iter_good && _args.markdown.at(_args.iterator) != '`') _args.iterator++;
	
	_args.html_out << _args.marks.code_marks.first;
	dump_block(_args);
	_args.html_out << _args.marks.code_marks.second;

	_args.block_begin = _args.iterator + 1;
}

void markdown_parsing::parse_code_block(args& _args)
{
	//Get past whitespaces
	while (iter_good && (_args.markdown.at(_args.iterator) == ' ' || _args.markdown.at(_args.iterator) == '\t'))
		_args.iterator++;

	//Get language name
	_args.block_begin = _args.iterator;

	while (
		_args.markdown.at(_args.iterator) != ' ' &&
		_args.markdown.at(_args.iterator) != '\n' &&
		_args.markdown.at(_args.iterator) != '\t'
	) _args.iterator++;

	std::string language_name = _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin);
	
	//Iterate till ```
	_args.block_begin = _args.iterator;

	while (iter_good)
	{
		if (_args.markdown.at(_args.iterator) == '`')
		{
			size_t amount = count_char('`', _args);
			if (amount == 3) break;
		}
		_args.iterator++;
	}

	_args.html_out << _args.marks.code_block_marks.first;

	_args.iterator -= 3;
	dump_block(_args);
	_args.iterator += 3;

	_args.html_out << _args.marks.code_block_marks.second;

	_args.block_begin = _args.iterator;
}

void markdown_parsing::parse_horizontal_rule(args& _args)
{
	_args.html_out << _args.marks.horizontal_rule;
	_args.iterator--;
	_args.block_begin = _args.iterator;
}

void markdown_parsing::parse_list(args& _args, bool ordered)
{
	//If there are no opened list or this item is nested, then push a new list and this item
	if (_args.ongoing_lists.size() == 0 || _args.ongoing_lists.back().items_indentation < _args.line_indentation)
	{
		//Calculate diffrence in indentation level between this and previous list
		size_t indentation_diffrence = _args.line_indentation - (_args.ongoing_lists.size() == 0 ? 0 : _args.ongoing_lists.back().items_indentation);

		_args.ongoing_lists.push_back(
			{
				.ordered = ordered,
				.items_indentation = _args.line_indentation,
				.indentation_diffrence = indentation_diffrence
			}
		);

		const html_marks::marks_pair* list_marks = ordered ?
			&_args.marks.ordered_list_marks :
			&_args.marks.unordered_list_marks;

		const html_marks::marks_pair* list_item_marks = ordered ?
			&_args.marks.ordered_list_item_marks :
			&_args.marks.unordered_list_item_marks;

		_args.html_out << list_marks->first;		 //Open list
		_args.html_out << list_item_marks->first;  //Open list item
	}
	//There are some list and their indentation is greater, then leave those lists
	else if (_args.ongoing_lists.back().items_indentation > _args.line_indentation)
	{
		size_t current_list_indentation = _args.ongoing_lists.back().items_indentation;

		//Remove all that are nested deeper
		while (_args.ongoing_lists.size() != 1 && current_list_indentation > _args.line_indentation)
		{
			current_list_indentation -= _args.ongoing_lists.back().indentation_diffrence;

			const html_marks::marks_pair* list_marks	= _args.ongoing_lists.back().ordered ?
				&_args.marks.ordered_list_marks :
				&_args.marks.unordered_list_marks;

			const html_marks::marks_pair* list_item_marks = _args.ongoing_lists.back().ordered ?
				&_args.marks.ordered_list_item_marks :
				&_args.marks.unordered_list_item_marks;

			_args.html_out << list_item_marks->second;	//Close list item
			_args.html_out << list_marks->second;			//Close list

			_args.ongoing_lists.pop_back();
		}

		//Push this item
		const html_marks::marks_pair* list_item_marks = _args.ongoing_lists.back().ordered ?
			&_args.marks.ordered_list_item_marks :
			&_args.marks.unordered_list_item_marks;

		_args.html_out << list_item_marks->second; //Close previous item
		_args.html_out << list_item_marks->first;	 //Open next item
	}
	//Else (there is a list and this item is in the same scope) simply push this item
	else
	{
		const html_marks::marks_pair* list_item_marks = _args.ongoing_lists.back().ordered ?
			&_args.marks.ordered_list_item_marks :
			&_args.marks.unordered_list_item_marks;

		_args.html_out << list_item_marks->second; //Close previous item
		_args.html_out << list_item_marks->first;	 //Open next item
	}

	//Skip the '-'
	_args.iterator++;
	_args.block_begin++;
}

void markdown_parsing::parse_simple_link(args& _args)
{
	//Skip <
	_args.iterator++;

	_args.block_begin = _args.iterator;
	while (iter_good && _args.markdown.at(_args.iterator) != '>') _args.iterator++;
	std::string link = _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin);
	
	_args.html_out << _args.marks.link_additional_marks.first;

	_args.html_out << "<a href=\"";
	_args.html_out << link;
	_args.html_out << "\">";
	_args.html_out << link;
	_args.html_out << "</a>";

	_args.html_out << _args.marks.link_additional_marks.second;

	_args.block_begin = _args.iterator;
}

void markdown_parsing::parse_named_link(args& _args)
{
	//Skip [
	_args.iterator++;

	//Get title
	_args.block_begin = _args.iterator;
	while (iter_good && _args.markdown.at(_args.iterator) != ']') _args.iterator++;
	std::string title = _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin);


	if (_args.markdown.at(_args.iterator) != ']') return;

	//Skip ]
	_args.iterator++;

	if (!iter_good || _args.markdown.at(_args.iterator) != '(')
		return;

	//Skip )
	_args.iterator++;
	if (!iter_good) return;

	//Get link
	_args.block_begin = _args.iterator;
	while (iter_good && _args.markdown.at(_args.iterator) != ')') _args.iterator++;
	std::string link = _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin);


	_args.html_out << _args.marks.link_additional_marks.first;

	_args.html_out << "<a href=\"";
	_args.html_out << link;
	_args.html_out << "\">";
	_args.html_out << title;
	_args.html_out << "</a>";

	_args.html_out << _args.marks.link_additional_marks.second;

	_args.block_begin = _args.iterator;
}

void markdown_parsing::parse_image(args& _args)
{
	//Skip !
	_args.iterator++;

	get_indentation(_args);

	if (!iter_good || _args.markdown.at(_args.iterator) != '[')
	{
		_args.iterator--;
		_args.block_begin = _args.iterator;
		return;
	}

	//Skip [
	_args.iterator++;

	//Get alt_text
	_args.block_begin = _args.iterator;
	while (iter_good && _args.markdown.at(_args.iterator) != ']') _args.iterator++;
	std::string alt_text = _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin);

	if (_args.markdown.at(_args.iterator) != ']') return;

	//Skip ]
	_args.iterator++;

	if (!iter_good || _args.markdown.at(_args.iterator) != '(')
		return;

	//Skip )
	_args.iterator++;
	if (!iter_good) return;

	//Get file
	_args.block_begin = _args.iterator;
	while (iter_good && _args.markdown.at(_args.iterator) != ')') _args.iterator++;
	std::string file = _args.markdown.substr(_args.block_begin, _args.iterator - _args.block_begin);


	_args.html_out << _args.marks.image_additional_marks.first;

	_args.html_out << "<img src=\"";
	_args.html_out << file;
	_args.html_out << "\" alt=\"";
	_args.html_out << alt_text;
	_args.html_out << "\">";

	_args.html_out << _args.marks.image_additional_marks.second;

	_args.block_begin = _args.iterator;
}

#undef iter_good

#endif // Markdown_Parser_Implementation
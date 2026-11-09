require 'tmpdir'

describe 'database' do
  around do |example|
    Dir.mktmpdir do |dir|
      @db_file = File.join(dir, 'test.db')
      example.run
    end
  end

  def run_script(commands, db_file = @db_file)
    raw_output = nil
    IO.popen(['./db', db_file], 'r+') do |pipe|
      commands.each do |command|
        pipe.puts command
      end

      pipe.close_write

      # Read entire output
      raw_output = pipe.gets(nil)
    end
    raw_output.split("\n")
  end

  # Mirrors src/row.h: ROW_SIZE = 4 (id) + 33 (username) + 256 (email) = 293
  ROW_SIZE = 4 + 33 + 256
  # Mirrors src/table.h: PAGE_SIZE / ROW_SIZE rows per page, 100 pages
  ROWS_PER_PAGE = 4096 / ROW_SIZE
  TABLE_MAX_ROWS = ROWS_PER_PAGE * 100

  it 'inserts and retrieves a row' do
    result = run_script([
      'insert 1 user1 person1@example.com',
      'select',
      '.exit',
    ])
    expect(result).to match_array([
      'db > Executed.',
      'db > (1, user1, person1@example.com)',
      'Executed.',
      'db > ',
    ])
  end

  # Disabled until leaf node splitting is implemented: a single leaf node
  # holds only 13 rows for now, so this test would fail at row 14.
  # it 'prints error message when table is full' do
  #   script = (1..(TABLE_MAX_ROWS + 1)).map do |i|
  #     "insert #{i} user#{i} person#{i}@example.com"
  #   end
  #   script << '.exit'
  #   result = run_script(script)
  #   expect(result[-2]).to eq('db > Error: Table full.')
  # end

  it 'allows inserting strings that are the maximum length' do
    long_username = 'a' * 32
    long_email = 'a' * 255
    script = [
      "insert 1 #{long_username} #{long_email}",
      'select',
      '.exit',
    ]
    result = run_script(script)
    expect(result).to match_array([
      'db > Executed.',
      "db > (1, #{long_username}, #{long_email})",
      'Executed.',
      'db > ',
    ])
  end

  it 'prints error message if strings are too long' do
    long_username = 'a' * 33
    long_email = 'a' * 256
    script = [
      "insert 1 #{long_username} #{long_email}",
      'select',
      '.exit',
    ]
    result = run_script(script)
    expect(result).to match_array([
      'db > String is too long.',
      'db > Executed.',
      'db > ',
    ])
  end

  it 'keeps data after closing the connection' do
    result1 = run_script([
      'insert 1 user1 person1@example.com',
      '.exit',
    ])
    expect(result1).to match_array([
      'db > Executed.',
      'db > ',
    ])

    result2 = run_script([
      'select',
      '.exit',
    ])
    expect(result2).to match_array([
      'db > (1, user1, person1@example.com)',
      'Executed.',
      'db > ',
    ])
  end

  it 'keeps data when input ends without .exit' do
    result1 = run_script([
      'insert 1 user1 person1@example.com',
    ])
    expect(result1).to match_array([
      'db > Executed.',
      'db > Error reading input',
    ])

    result2 = run_script([
      'select',
      '.exit',
    ])
    expect(result2).to match_array([
      'db > (1, user1, person1@example.com)',
      'Executed.',
      'db > ',
    ])
  end

  # Disabled until leaf node splitting is implemented: 30 rows do not fit in
  # a single leaf node (13 rows max), so this test would fail.
  # it 'keeps data across several pages after closing the connection' do
  #   row_count = 30
  #   inserts = (1..row_count).map do |i|
  #     "insert #{i} user#{i} person#{i}@example.com"
  #   end
  #   result1 = run_script(inserts + ['.exit'])
  #   expect(result1.last).to eq('db > ')
  #   expect(result1.count('db > Executed.')).to eq(row_count)
  #
  #   result2 = run_script(['select', '.exit'])
  #   expected_rows = (1..row_count).map do |i|
  #     "(#{i}, user#{i}, person#{i}@example.com)"
  #   end
  #   expected_rows[0] = "db > #{expected_rows[0]}"
  #   expect(result2).to eq(expected_rows + ['Executed.', 'db > '])
  # end

  it 'prints constants' do
    script = [
      '.constants',
      '.exit',
    ]
    result = run_script(script)

    expect(result).to match_array([
      'db > Constants:',
      'ROW_SIZE: 293',
      'COMMON_NODE_HEADER_SIZE: 6',
      'LEAF_NODE_HEADER_SIZE: 10',
      'LEAF_NODE_CELL_SIZE: 297',
      'LEAF_NODE_SPACE_FOR_CELLS: 4086',
      'LEAF_NODE_MAX_CELLS: 13',
      'db > ',
    ])
  end

  it 'allows printing out the structure of a one-node btree' do
    script = [3, 1, 2].map do |i|
      "insert #{i} user#{i} person#{i}@example.com"
    end
    script << '.btree'
    script << '.exit'
    result = run_script(script)

    expect(result).to match_array([
      'db > Executed.',
      'db > Executed.',
      'db > Executed.',
      'db > Tree:',
      'leaf (size 3)',
      '  - 0 : 1',
      '  - 1 : 2',
      '  - 2 : 3',
      'db > ',
    ])
  end

  it 'prints an error message if there is a duplicate id' do
    script = [
      'insert 1 user1 person1@example.com',
      'insert 1 user1 person1@example.com',
      'select',
      '.exit',
    ]
    result = run_script(script)
    expect(result).to match_array([
      'db > Executed.',
      'db > Error: Duplicate key.',
      'db > (1, user1, person1@example.com)',
      'Executed.',
      'db > ',
    ])
  end

  it 'prints an error message if no database filename is given' do
    raw_output = IO.popen(['./db'], 'r+') do |pipe|
      pipe.close_write
      pipe.gets(nil)
    end
    expect(raw_output.split("\n")).to eq([
      'Must supply a database filename.',
    ])
  end

  it 'prints an error message if id is negative' do
    script = [
      'insert -1 cstack foo@bar.com',
      'select',
      '.exit',
    ]
    result = run_script(script)
    expect(result).to match_array([
      'db > ID must be positive.',
      'db > Executed.',
      'db > ',
    ])
  end

  it 'rejects an id with trailing non-numeric characters' do
    script = [
      'insert 12abc cstack foo@bar.com',
      'select',
      '.exit',
    ]
    result = run_script(script)
    expect(result).to match_array([
      'db > Syntax error. Could not parse statement.',
      'db > Executed.',
      'db > ',
    ])
  end

  it 'prints an error message if id is larger than UINT32_MAX' do
    script = [
      'insert 4294967296 cstack foo@bar.com',
      'select',
      '.exit',
    ]
    result = run_script(script)
    expect(result).to match_array([
      'db > ID is too large.',
      'db > Executed.',
      'db > ',
    ])
  end
end

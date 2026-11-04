describe 'database' do
  def run_script(commands)
    raw_output = nil
    IO.popen('./db', 'r+') do |pipe|
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

  it 'prints error message when table is full' do
    script = (1..(TABLE_MAX_ROWS + 1)).map do |i|
      "insert #{i} user#{i} person#{i}@example.com"
    end
    script << '.exit'
    result = run_script(script)
    expect(result[-2]).to eq('db > Error: Table full.')
  end

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

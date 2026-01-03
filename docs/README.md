# MetaImGUI Documentation

This directory contains documentation for the MetaImGUI project.

## API Documentation

API documentation is generated using Doxygen from source code comments.

### Generating Documentation

To generate the API documentation:

```bash
# Install Doxygen if not already installed
# Ubuntu/Debian:
sudo apt-get install doxygen graphviz

# macOS:
brew install doxygen graphviz

# Fedora:
sudo dnf install doxygen graphviz

# Generate documentation
cd /path/to/MetaImGUI
doxygen Doxyfile
```

The generated documentation will be in `docs/api/html/`. Open `docs/api/html/index.html` in your browser.

### Online Documentation

If you've set up GitHub Pages, the documentation will be automatically deployed and available at:
https://yourusername.github.io/MetaImGUI/

## Documentation Structure

```
docs/
├── README.md              # This file
├── api/                   # Generated API documentation (Doxygen)
│   └── html/             # HTML output
├── architecture.md        # System architecture overview
├── extending.md           # Guide for extending MetaImGUI
└── best-practices.md      # Best practices and patterns
```

## Writing Documentation

### Code Comments

Use Doxygen-style comments in header files:

```cpp
/**
 * @brief Brief description of the class
 *
 * Detailed description of what this class does,
 * how to use it, and any important notes.
 */
class MyClass {
public:
    /**
     * @brief Initialize the object
     *
     * @param parameter Description of parameter
     * @return true if successful, false otherwise
     */
    bool Initialize(int parameter);

private:
    int m_member; ///< Brief description of member variable
};
```

### Markdown Documentation

- Use clear, descriptive headings
- Include code examples where helpful
- Add diagrams for complex concepts
- Link to related documentation
- Keep examples up-to-date with code changes

## Documentation Guidelines

1. **API Documentation**:
   - Document all public interfaces
   - Include parameter descriptions
   - Specify return values
   - Note any exceptions or error conditions
   - Provide usage examples

2. **Guides and Tutorials**:
   - Start with the simplest use case
   - Build complexity gradually
   - Include complete, working examples
   - Explain why, not just how

3. **Architecture Documentation**:
   - Document design decisions
   - Explain component interactions
   - Include class diagrams
   - Describe threading model
   - Note performance considerations

## Contributing to Documentation

When contributing code:
- Update relevant documentation
- Add Doxygen comments to new public APIs
- Update examples if behavior changes
- Add new guides for significant features

See [CONTRIBUTING.md](../CONTRIBUTING.md) for more details.

## Building Documentation for GitHub Pages

To deploy documentation to GitHub Pages:

1. Generate documentation locally:
   ```bash
   doxygen Doxyfile
   ```

2. The output is in `docs/api/html/`

3. Configure GitHub Pages:
   - Go to repository Settings → Pages
   - Source: Deploy from a branch
   - Branch: gh-pages (create if needed)
   - Folder: / (root) or /docs

4. Push documentation:
   ```bash
   # Create gh-pages branch (first time only)
   git checkout --orphan gh-pages
   git reset --hard
   cp -r docs/api/html/* .
   git add .
   git commit -m "Update documentation"
   git push origin gh-pages
   git checkout main
   ```

## Documentation Tools

- **Doxygen**: API documentation generator
- **Graphviz**: Diagram generation (optional, for class diagrams)
- **Markdown**: For guides and tutorials
- **PlantUML**: For UML diagrams (optional)

## Additional Resources

- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Markdown Guide](https://www.markdownguide.org/)
- [GitHub Pages Documentation](https://docs.github.com/en/pages)

## Questions?

If you have questions about the documentation:
- Open an issue on GitHub
- See [CONTRIBUTING.md](../CONTRIBUTING.md)
- Contact the maintainers


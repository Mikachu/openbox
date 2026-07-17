//! Benchmarks for client operations

use criterion::{black_box, criterion_group, criterion_main, Criterion, BenchmarkId};
use openbox_core::client::{ClientManager, WindowType, WindowState, ClientId};
use openbox_core::types::{WindowHandle, Geometry};

fn bench_add_clients(c: &mut Criterion) {
    let mut group = c.benchmark_group("add_clients");
    
    for count in [10, 100, 1000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(count), count, |b, &count| {
            b.iter(|| {
                let mut clients = ClientManager::new();
                for i in 0..count {
                    let _ = clients.add_client(
                        WindowHandle::Generic(i as u64),
                        WindowType::Normal,
                    );
                }
                black_box(clients);
            });
        });
    }
    
    group.finish();
}

fn bench_client_lookup(c: &mut Criterion) {
    let mut clients = ClientManager::new();
    let ids: Vec<ClientId> = (0..1000)
        .map(|i| clients.add_client(WindowHandle::Generic(i), WindowType::Normal).unwrap())
        .collect();
    
    c.bench_function("client_lookup_by_id", |b| {
        b.iter(|| {
            for id in &ids {
                black_box(clients.get(*id));
            }
        });
    });
    
    c.bench_function("client_lookup_by_handle", |b| {
        b.iter(|| {
            for i in 0..1000 {
                black_box(clients.get_by_handle(WindowHandle::Generic(i)));
            }
        });
    });
}

fn bench_state_operations(c: &mut Criterion) {
    let mut clients = ClientManager::new();
    let ids: Vec<ClientId> = (0..100)
        .map(|i| clients.add_client(WindowHandle::Generic(i), WindowType::Normal).unwrap())
        .collect();
    
    c.bench_function("set_state", |b| {
        b.iter(|| {
            for id in &ids {
                clients.set_state(*id, WindowState::Maximized, true).unwrap();
            }
        });
    });
    
    c.bench_function("check_state", |b| {
        b.iter(|| {
            for id in &ids {
                black_box(clients.has_state(*id, WindowState::Maximized));
            }
        });
    });
}

criterion_group!(benches, bench_add_clients, bench_client_lookup, bench_state_operations);
criterion_main!(benches);
